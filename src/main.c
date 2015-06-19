/* To fix the window problems that we were having, I moved the fuctions for the bitmap's (bitmap_name = bitmap_create(Grect(x,y,x,y)) - lines 334-336, line 171) from the init() function of the program to the ***_window_load() funciton.
  I think what was happening was, even though the bitmaps were created in the init() function when the app was first opened, I think the individual windows were having trouble accessing them.
  Creating the bitmaps upon window load makes it so they are always accessible when the window is pushed to the top of the stack.
  Even though I originally though the app was crashing due to memory problems, it was probably becasue it was trying to reference a variable that it just couldn't find.
  Oh, we also forgot to have gbitmap_destroy() functions, that's another reason the windows were causing a crash.
  */

#define KEY_BALANCE_DOLLARS 12
  #define KEY_BALANCE_CENTS 13
  #define KEY_LETTER_1 14
  #define KEY_LETTER_2 15
  #define KEY_LETTER_3 16
  #define KEY_NAME_STORE 17

  #include <pebble.h>
  #include <stdlib.h>
  #include <string.h>

  char balance_buffer[100];
double balance_total = 20.00;  //when we're calculating the balance after a transaction, just subtract from this var.
int balance_dollars;  // these are only used when printing to the screen
int balance_cents;    // (so don't actually set any value to these)

char order_total_buffer[100];
double order_total; //same with this variable.
double temp;
int order_dollars;  // and these
int order_cents;    // this too

typedef struct order_details
{
  char item[50];
  char item_size[50];
} order_details;

order_details order[10];

int order_count = 0;


// -----------------------------------------------------------------------
// **Lets try to keep all variables for storing the order above this line**

// I split all of the variables up into sections, all of the variables in each section are related to each other (they are in the same window, layer, etc.)
// I also tried to organize it in the order that it will be viewed

static Window *main_window;   //Main window that loads when the app is opened
static TextLayer *header_layer;  //header used for main window
static MenuLayer *main_menu_layer;
#define NUM_FIRST_MENU_ITEMS 5

  static Window *map_window;    // holds the map, will eventually try to make it so we can pan the map picture around instead of it being static
static BitmapLayer *map_layer;
static GBitmap *map_bitmap;

static Window *balance_window;
static TextLayer *balance_text_layer;

static Window *name_window;
static TextLayer *name_text_layer_1;
static TextLayer *name_text_layer_2;
static TextLayer *name_text_layer_3;
static BitmapLayer *name_bitmap_layer;
static GBitmap *name_bitmap1;
const char alphabet_string[28] = "-ABCDEFGHIJKLMNOPQRSTUVWXYZ-";
int letter_count_1 = 0, letter_count_2 = 0, letter_count_3 = 0;
char letter_buffer_1;
char letter_buffer_2;
char letter_buffer_3;
char name[3];
int select_count = 0;

static Window *order_window;  //contains all (?) of the items that can be ordered in different sections
static MenuLayer *order_menu_layer;
#define NUM_ORDER_MENU_1_ITEMS 4    //these three values are unique to this menu layer
  #define NUM_ORDER_MENU_2_ITEMS 6
  #define NUM_ORDER_MENU_SECTIONS 2

  static Window *drink_specifics_window;  //Let's use this for drink specifics (size)
static MenuLayer *drink_specifics_menu_layer;
static TextLayer *drink_specifics_text_layer;
#define NUM_DRINK_MENU_ITEMS 3

  static Window *confirmation_window; //will be used to display the total and the order status
static MenuLayer *confirmation_menu_layer;
static TextLayer *confirmation_text_layer;
#define NUM_CONFIRMATION_MENU_ITEMS 2;

static Window *order_submitted_window;
static TextLayer *order_submitted_text_layer;

//------------------------------------
//------------------------------------
//------------------------------------
//------------------------------------

#define KEY_ORDER_TOTAL 0
  #define KEY_NAME 1
  //item keys are 2-6
  //item size keys are 7-11

  DictionaryIterator *iterator;

static void send_order() {
  DictionaryIterator* iter;
  app_message_outbox_begin(&iter);

  char order_total_send[10];
  snprintf(order_total_send, 10, "$%d.%02d", order_dollars, order_cents);

  strncpy(&letter_buffer_1, &alphabet_string[letter_count_1],1);
  strncpy(&letter_buffer_2, &alphabet_string[letter_count_2],1);
  strncpy(&letter_buffer_3, &alphabet_string[letter_count_3],1);
  snprintf(name, 4, "%c%c%c", letter_buffer_1, letter_buffer_2, letter_buffer_3);

  dict_write_cstring(iter, KEY_ORDER_TOTAL, order_total_send);  
  dict_write_cstring(iter, KEY_NAME, name);  

  for (int i=0; i<=order_count; i++)
  {
    dict_write_cstring(iter, (i+2), order[i].item);  
    dict_write_cstring(iter, (i+7), order[i].item_size);  
  }
  dict_write_end(iter);
  app_message_outbox_send();
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

//------------------------------------
//------------------------------------
//------------------------------------
//------------------------------------

// *** START MAIN WINDOW FUNCTIONS ***

// So this is where it could get confusing. When building menus, you cannot just say have a variable equal to the number of rows that you plug into a function.
// All of the menu functions are defined as a certain structrure and have to be set up in the following way:


//This function simply returns the value that is defined above for the amount of rows in the menu
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
    return NUM_FIRST_MENU_ITEMS;
    default:
    return 0;
  }
}

//This actually draws the titles or subtitles to the text boxes used in the menu
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
    switch (cell_index->row) {
      case 0:
      menu_cell_basic_draw(ctx, cell_layer, "Place Order", NULL, NULL);
      break;
      case 1:
      menu_cell_basic_draw(ctx, cell_layer, "Check Card", "Balance", NULL);
      break;
      case 2:
      menu_cell_basic_draw(ctx, cell_layer, "Find Closest", "BariBucks", NULL);
      break;
      case 3:
      menu_cell_basic_draw(ctx, cell_layer, "Change Name", NULL, NULL);
      break;
      case 4:
      menu_cell_basic_draw(ctx, cell_layer, "Reload Balance", NULL, NULL);
      break;

    }
    break;
  }
}

//This function actually performs the action when a menu is clicked
static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case 0:                  //Case 0: (first menu) is the order menu item
    order_total = 0;
    order_count = 0;
    window_stack_push(order_window, true);  //...so the order window is pushed
    break;
    case 1:                                      //Case 1: (second menu) is the balance menu item
    window_stack_push(balance_window, true);  //balance window pushed to top
    break;
    case 2:                                      //Case 2: (third menu) is the map menu item
    window_stack_push(map_window, true);  //map window pushed to top
    break;
    case 3:   
    select_count = 1;
    window_stack_push(name_window, true);
    break;
    case 4:   
    balance_total = 50;
    break;
  }
}

static void main_window_load(Window *window) {

  Layer *window_layer = window_get_root_layer(window);
  header_layer = text_layer_create(GRect(0, 0, 144, 40));
  text_layer_set_font(header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(header_layer, GTextAlignmentCenter);
  text_layer_set_text_color(header_layer, GColorBlack);
  text_layer_set_background_color(header_layer, GColorClear);
  text_layer_set_text(header_layer, "BariBucks");
  layer_add_child(window_layer, text_layer_get_layer(header_layer));

  main_menu_layer = menu_layer_create(GRect(0, 40, 144, 113));
  menu_layer_set_callbacks(main_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = menu_get_num_rows_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback, //the .(after the dot) is predefined
  });

  menu_layer_set_click_config_onto_window(main_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(main_menu_layer));
}

static void main_window_unload(Window *window) {
  menu_layer_destroy(main_menu_layer);
  text_layer_destroy(header_layer);
}

// *** END MAIN WINDOW FUNCTIONS ***

//------------------------------------
//------------------------------------
//------------------------------------

// *** START MAP WINDOW FUNCTIONS ***

static void map_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  map_bitmap = gbitmap_create_with_resource(RESOURCE_ID_map);

  map_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(map_layer, map_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(map_layer));
}

static void map_window_unload(Window *window) {
  bitmap_layer_destroy(map_layer);
  gbitmap_destroy(map_bitmap);
}
// *** END MAP WINDOW FUNCTIONS ***

//------------------------------------
//------------------------------------
//------------------------------------

// *** START BALANCE WINDOW FUNCTIONS ***

static void balance_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  balance_dollars = balance_total;
  balance_cents = (balance_total * 100) - (balance_dollars * 100);

  snprintf(balance_buffer, 100, "\nYour current balance is $%d.%02d.", balance_dollars, balance_cents);

  balance_text_layer = text_layer_create(GRect(0, 0, 144, 168));
  text_layer_set_text_alignment(balance_text_layer, GTextAlignmentCenter);
  text_layer_set_text_color(balance_text_layer, GColorBlack);
  text_layer_set_background_color(balance_text_layer, GColorClear);
  text_layer_set_text(balance_text_layer, balance_buffer);
  text_layer_set_font(balance_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  layer_add_child(window_layer, text_layer_get_layer(balance_text_layer));
}

static void balance_window_unload(Window *window) {
  text_layer_destroy(balance_text_layer);
}

// *** END BALANCE WINDOW FUNCTIONS ***

//------------------------------------
//------------------------------------
//------------------------------------
// *** START NAME WINDOW FUNCTIONS ***

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (select_count) {
    case 1:
    {
      if (letter_count_1 == 26)
        letter_count_1 = -1; 
      letter_count_1++;
      strncpy(&letter_buffer_1, &alphabet_string[letter_count_1],1);
      text_layer_set_text(name_text_layer_1, &letter_buffer_1);
      break;
    }
    case 2:
    {
      if (letter_count_2 == 26)
        letter_count_2 = -1; 
      letter_count_2++;
      strncpy(&letter_buffer_2, &alphabet_string[letter_count_2],1);
      text_layer_set_text(name_text_layer_2, &letter_buffer_2);
      break;
    }
    case 3:
    {
      if (letter_count_3 == 26)
        letter_count_3 = -1; 
      letter_count_3++;
      strncpy(&letter_buffer_3, &alphabet_string[letter_count_3],1);
      text_layer_set_text(name_text_layer_3, &letter_buffer_3);
      break;
    } 
    default: 
    break; 
  } 
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {

  switch (select_count)
  {
    case 1:
    text_layer_set_text_color(name_text_layer_1, GColorBlack);
    text_layer_set_background_color(name_text_layer_1, GColorWhite);
    text_layer_set_text_color(name_text_layer_2, GColorWhite);
    text_layer_set_background_color(name_text_layer_2, GColorBlack);
    break;
    case 2:
    text_layer_set_text_color(name_text_layer_2, GColorBlack);
    text_layer_set_background_color(name_text_layer_2, GColorWhite);
    text_layer_set_text_color(name_text_layer_3, GColorWhite);
    text_layer_set_background_color(name_text_layer_3, GColorBlack);
    break;
    case 3:
    {
      text_layer_set_text_color(name_text_layer_1, GColorWhite);
      text_layer_set_background_color(name_text_layer_1, GColorBlack);
      text_layer_set_text_color(name_text_layer_3, GColorWhite);
      text_layer_set_background_color(name_text_layer_3, GColorBlack);

      bitmap_layer_set_bitmap(name_bitmap_layer, name_bitmap1);
      select_count = 0;
      window_stack_remove(name_window, true);
      snprintf(name, 4, "%c%c%c", letter_buffer_1, letter_buffer_2, letter_buffer_3);
      APP_LOG(APP_LOG_LEVEL_INFO, " Name: %s ", name);
    }
    break;
    default:
    bitmap_layer_set_bitmap(name_bitmap_layer, name_bitmap1);
    break;
  }
  select_count++;
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  select_count--;

  switch (select_count)
  {
    case 1:
    text_layer_set_text_color(name_text_layer_1, GColorWhite);
    text_layer_set_background_color(name_text_layer_1, GColorBlack);
    text_layer_set_text_color(name_text_layer_2, GColorBlack);
    text_layer_set_background_color(name_text_layer_2, GColorWhite);

    break;
    case 2:
    text_layer_set_text_color(name_text_layer_2, GColorWhite);
    text_layer_set_background_color(name_text_layer_2, GColorBlack);
    text_layer_set_text_color(name_text_layer_3, GColorBlack);
    text_layer_set_background_color(name_text_layer_3, GColorWhite);
    break;
    case 0:
    {
      text_layer_set_text_color(name_text_layer_1, GColorWhite);
      text_layer_set_background_color(name_text_layer_1, GColorBlack);
      text_layer_set_text_color(name_text_layer_3, GColorBlack);
      text_layer_set_background_color(name_text_layer_3, GColorWhite);

      bitmap_layer_set_bitmap(name_bitmap_layer, name_bitmap1);
      select_count = 0;
      window_stack_remove(name_window, true);
    }
    break;
  }
}


static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (select_count)
  {
    case 1:
    {      if (letter_count_1 == 0)
      letter_count_1 = 27; 

     letter_count_1--;
     strncpy(&letter_buffer_1, &alphabet_string[letter_count_1],1);
     text_layer_set_text(name_text_layer_1, &letter_buffer_1);
     break;
    }
    case 2:
    {      if (letter_count_2 == 0)
      letter_count_2 = 27; 

     letter_count_2--;
     strncpy(&letter_buffer_2, &alphabet_string[letter_count_2],1);
     text_layer_set_text(name_text_layer_2, &letter_buffer_2);
     break;
    }
    case 3:
    {      if (letter_count_3 == 0)
      letter_count_3 = 27; 

     letter_count_3--;
     strncpy(&letter_buffer_3, &alphabet_string[letter_count_3],1);
     text_layer_set_text(name_text_layer_3, &letter_buffer_3);
     break;
    } 
    default: 
    break; 
  }
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void name_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  window_set_click_config_provider(name_window, click_config_provider);  
  name_bitmap1 = gbitmap_create_with_resource(RESOURCE_ID_name_window_1);

  strncpy(&letter_buffer_1, &alphabet_string[letter_count_1],1);
  strncpy(&letter_buffer_2, &alphabet_string[letter_count_2],1);
  strncpy(&letter_buffer_3, &alphabet_string[letter_count_3],1);

  name_bitmap_layer = bitmap_layer_create(GRect(0, -10, 144, 100));
  bitmap_layer_set_bitmap(name_bitmap_layer, name_bitmap1);
  layer_add_child(window_layer, bitmap_layer_get_layer(name_bitmap_layer));

  name_text_layer_1 = text_layer_create(GRect(5, 90, 45 , 50));
  text_layer_set_font(name_text_layer_1, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_color(name_text_layer_1, GColorWhite);
  text_layer_set_background_color(name_text_layer_1, GColorBlack);
  text_layer_set_text_alignment(name_text_layer_1, GTextAlignmentCenter);
  text_layer_set_text(name_text_layer_1, &letter_buffer_1);
  layer_add_child(window_layer, text_layer_get_layer(name_text_layer_1));

  name_text_layer_2 = text_layer_create(GRect(50, 90, 45 , 50));
  text_layer_set_font(name_text_layer_2, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_color(name_text_layer_2, GColorBlack);
  text_layer_set_background_color(name_text_layer_2, GColorWhite);
  text_layer_set_text_alignment(name_text_layer_2, GTextAlignmentCenter);
  text_layer_set_text(name_text_layer_2, &letter_buffer_2);
  layer_add_child(window_layer, text_layer_get_layer(name_text_layer_2));

  name_text_layer_3 = text_layer_create(GRect(95, 90, 45 , 50));
  text_layer_set_font(name_text_layer_3, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_color(name_text_layer_3, GColorBlack);
  text_layer_set_background_color(name_text_layer_3, GColorWhite);
  text_layer_set_text_alignment(name_text_layer_3, GTextAlignmentCenter);
  text_layer_set_text(name_text_layer_3, &letter_buffer_3);
  layer_add_child(window_layer, text_layer_get_layer(name_text_layer_3));
}

static void name_window_unload(Window *window) {
  text_layer_destroy(name_text_layer_1);
  text_layer_destroy(name_text_layer_2);
  text_layer_destroy(name_text_layer_3);
  bitmap_layer_destroy(name_bitmap_layer);
  gbitmap_destroy(name_bitmap1);
}

// *** END NAME WINDOW FUNCTIONS ***
//------------------------------------
//------------------------------------
//------------------------------------

// *** START ORDER WINDOW FUNCTIONS ***

static uint16_t order_menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_ORDER_MENU_SECTIONS;
}

static int16_t order_menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void order_menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
    menu_cell_basic_header_draw(ctx, cell_layer, "Drinks");
    break;
    case 1:
    menu_cell_basic_header_draw(ctx, cell_layer, "Food");
    break;
  }
}

static uint16_t order_menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
    return NUM_ORDER_MENU_1_ITEMS;
    case 1:
    return NUM_ORDER_MENU_2_ITEMS;
    default:
    return 0;
  }
}

static void order_menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
    switch (cell_index->row) {
      case 0:
      menu_cell_basic_draw(ctx, cell_layer, "Hot Coffee", NULL, NULL);
      break;
      case 1:
      menu_cell_basic_draw(ctx, cell_layer, "Iced Coffee", NULL, NULL);
      break;
      case 2:
      menu_cell_basic_draw(ctx, cell_layer, "Tea", NULL, NULL);
      break;
      case 3:
      menu_cell_basic_draw(ctx, cell_layer, "Frappuccino", NULL, NULL);
      break;
    }
    break;
    case 1:
    switch (cell_index->row) {
      case 0:
      menu_cell_basic_draw(ctx, cell_layer, "Sandwich", "Turkey", NULL);
      break;
      case 1:
      menu_cell_basic_draw(ctx, cell_layer, "Sandwich", "Ham", NULL);
      break;
      case 2:
      menu_cell_basic_draw(ctx, cell_layer, "Muffin", "Blueberry", NULL);
      break;
      case 3:
      menu_cell_basic_draw(ctx, cell_layer, "Muffin", "Chocolate chip", NULL);
      break;
      case 4:
      menu_cell_basic_draw(ctx, cell_layer, "Cookie", "Rasin", NULL);
      break;
      case 5:
      menu_cell_basic_draw(ctx, cell_layer, "Cookie", "Chocolate chip", NULL);
      break;
    }
    break;
  }
}

static void order_menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
    switch (cell_index->row) {
      case 0:
      strcpy(order[order_count].item, "Hot Coffee");
      order_total = order_total + 2.19;
      window_stack_push(drink_specifics_window, true);  //push window to choose size
      break;
      case 1:
      strcpy(order[order_count].item, "Iced Coffee");
      order_total = order_total + 2.29;
      window_stack_push(drink_specifics_window, true);  //push window to choose size
      break;
      case 2:
      strcpy(order[order_count].item, "Tea");
      order_total = order_total + 1.89;
      window_stack_push(drink_specifics_window, true);  //push window to choose size
      break;
      case 3:
      strcpy(order[order_count].item, "Frappuccino");
      order_total = order_total + 3.89;
      window_stack_push(drink_specifics_window, true);  //push window to choose size
      break;
    }
    break;
    case 1:
    switch (cell_index->row) {
      case 0:
      strcpy(order[order_count].item, "Turkey Sandwich");
      order_total = order_total + 4.55;
      window_stack_push(confirmation_window, true);
      break;
      case 1:
      strcpy(order[order_count].item, "Ham Sandwich");
      order_total = order_total + 4.55;
      window_stack_push(confirmation_window, true);
      break;
      case 2:
      strcpy(order[order_count].item, "Blueberry Muffin");
      order_total = order_total + 1.99;
      window_stack_push(confirmation_window, true);
      break;
      case 3:
      strcpy(order[order_count].item, "Chocolate Chip Muffin");
      order_total = order_total + 1.99;
      window_stack_push(confirmation_window, true);
      break;
      case 4:
      strcpy(order[order_count].item, "Rasin Cookie");
      order_total = order_total + 0.99;
      window_stack_push(confirmation_window, true);
      break;
      case 5:
      strcpy(order[order_count].item, "Chocolate Chip Cookie");
      order_total = order_total + 0.99;
      window_stack_push(confirmation_window, true);
      break;
    }
    break;
  }
}


static void order_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  order_menu_layer = menu_layer_create(GRect(0, 0, 144, 153));
  menu_layer_set_callbacks(order_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_sections = order_menu_get_num_sections_callback,
    .get_num_rows = order_menu_get_num_rows_callback,
    .get_header_height = order_menu_get_header_height_callback,
    .draw_header = order_menu_draw_header_callback,
    .draw_row = order_menu_draw_row_callback,
    .select_click = order_menu_select_callback,
  });

  menu_layer_set_click_config_onto_window(order_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(order_menu_layer));
}

static void order_window_unload(Window *window) {
  menu_layer_destroy(order_menu_layer);
}

// *** END ORDER WINDOW FUNCTIONS ***

//------------------------------------
//------------------------------------
//------------------------------------

// *** START DRINK SPECIFICS WINDOW FUNCTIONS ***

static uint16_t drink_menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
    return NUM_DRINK_MENU_ITEMS;
    default:
    return 0;
  }
}

static void drink_menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
    switch (cell_index->row) {
      case 0:
      menu_cell_basic_draw(ctx, cell_layer, "Tall", NULL, NULL);
      break;
      case 1:
      menu_cell_basic_draw(ctx, cell_layer, "Grande", NULL, NULL);
      break;
      case 2:
      menu_cell_basic_draw(ctx, cell_layer, "Venti", NULL, NULL);
      break;
    }
    break;
  }
}

static void drink_menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case 0:
    strcpy(order[order_count].item_size, "Tall");
    order_total = order_total + 0;
    window_stack_push(confirmation_window, true);  
    break;
    case 1:
    strcpy(order[order_count].item_size, "Grande");
    order_total = order_total + 0.20;
    window_stack_push(confirmation_window, true); 
    break;
    case 2:    
    strcpy(order[order_count].item_size, "Venti");
    order_total = order_total + 0.45;
    window_stack_push(confirmation_window, true);  
    break;
  }
}

static void drink_window_load(Window *window) {

  Layer *window_layer = window_get_root_layer(window);

  drink_specifics_text_layer = text_layer_create(GRect(0, 0, 144, 30));
  text_layer_set_font(drink_specifics_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(drink_specifics_text_layer, GTextAlignmentCenter);
  text_layer_set_text_color(drink_specifics_text_layer, GColorBlack);
  text_layer_set_background_color(drink_specifics_text_layer, GColorClear);
  text_layer_set_text(drink_specifics_text_layer, "Choose a size:");
  layer_add_child(window_layer, text_layer_get_layer(drink_specifics_text_layer));

  drink_specifics_menu_layer = menu_layer_create(GRect(0, 30, 144, 123));
  menu_layer_set_callbacks(drink_specifics_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = drink_menu_get_num_rows_callback,
    .draw_row = drink_menu_draw_row_callback,
    .select_click = drink_menu_select_callback, //the .(after the dot) is predefined
  });

  menu_layer_set_click_config_onto_window(drink_specifics_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(drink_specifics_menu_layer));
}

static void drink_window_unload(Window *window) {
  menu_layer_destroy(drink_specifics_menu_layer);
  text_layer_destroy(drink_specifics_text_layer);
}


// *** END DRINK SPECIFICS WINDOW FUNCTIONS ***

//------------------------------------
//------------------------------------
//------------------------------------

// *** START CONFIRMATION WINDOW FUNCTIONS ***

static uint16_t confirmation_menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
    return NUM_CONFIRMATION_MENU_ITEMS;
    default:
    return 0;
  }
}

static void confirmation_menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case 0:
    menu_cell_basic_draw(ctx, cell_layer, "Yes", NULL, NULL);
    break;
    case 1:
    menu_cell_basic_draw(ctx, cell_layer, "No", NULL, NULL);
    break;
  }
}

static void confirmation_menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case 0:
    order_count++;
    if (order_count == 5 && order_total <= balance_total)
    {
      order_dollars = order_total;
      order_cents = (order_total * 100) - (order_dollars * 100);

      snprintf(order_total_buffer, 200, "*Max 5 items reached* Order Submitted!\n\nOrder total: $%d.%02d.\n\nPress the back button to start over.", order_dollars, order_cents);
      balance_total = balance_total - order_total;
      send_order();
      window_stack_push(order_submitted_window, true);
    }
    else if (order_count == 5 && order_total >= balance_total)
    {        
      order_dollars = order_total;
      order_cents = (order_total * 100) - (order_dollars * 100);
      snprintf(order_total_buffer, 200, "\nInsufficient funds!\n\nYou need $%d.%02d more.\n\nPress the back button to add more funds.", order_dollars-balance_dollars, order_cents-balance_cents);
      order_total = 0;
      window_stack_push(order_submitted_window, true);
    }
    else
      window_stack_push(order_window, true);
    break;
    case 1:
    order_dollars = order_total;
    order_cents = (order_total * 100) - (order_dollars * 100);
    if (order_total <= balance_total) {
      snprintf(order_total_buffer, 200, "Your order has been submitted!\n\nOrder total: $%d.%02d.\n\nPress the back button to start over.", order_dollars, order_cents);
      balance_total = balance_total - order_total;
      send_order();
    }
    else {
      snprintf(order_total_buffer, 200, "\nInsufficient funds!\n\nYou need $%d.%02d more.\n\nPress the back button to add more funds.", order_dollars-balance_dollars, order_cents-balance_cents);
      order_total = 0;
    }
    window_stack_push(order_submitted_window, true);
    break;
  }
}

static void confirmation_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  confirmation_text_layer = text_layer_create(GRect(0, 0, 144, 50));
  text_layer_set_font(confirmation_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_color(confirmation_text_layer, GColorBlack);
  text_layer_set_background_color(confirmation_text_layer, GColorClear);
  text_layer_set_text(confirmation_text_layer, "Anything Else?");
  layer_add_child(window_layer, text_layer_get_layer(confirmation_text_layer));

  confirmation_menu_layer = menu_layer_create(GRect(0, 50, 144, 108));
  menu_layer_set_callbacks(confirmation_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = confirmation_menu_get_num_rows_callback,
    .draw_row = confirmation_menu_draw_row_callback,
    .select_click = confirmation_menu_select_callback,
  });

  menu_layer_set_click_config_onto_window(confirmation_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(confirmation_menu_layer));
}

static void confirmation_window_unload(Window *window) {
  menu_layer_destroy(confirmation_menu_layer);
  text_layer_destroy(confirmation_text_layer);
}

// *** END CONFIRMATION WINDOW FUNCTIONS ***

//------------------------------------
//------------------------------------
//------------------------------------

// *** START SUBMITTED WINDOW FUNCTIONS ***
static void submitted_back_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_pop_all(false);
  window_stack_push(main_window, true);
}

static void submitted_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  psleep(7500);
  snprintf(order_total_buffer, 200, "Your order is ready! Proceed to the counter to pick it up.\n\nBack button returns to the main menu.");
  text_layer_set_text(order_submitted_text_layer, order_total_buffer);
  text_layer_set_font(order_submitted_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24)); 
  vibes_double_pulse();

}

static void submitted_click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_BACK, submitted_back_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, submitted_select_click_handler);

}

static void order_submitted_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  window_set_click_config_provider(order_submitted_window, submitted_click_config_provider);  

  order_submitted_text_layer = text_layer_create(GRect(0, 0, 144, 168));
  text_layer_set_font(order_submitted_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(order_submitted_text_layer, GTextAlignmentCenter);
  text_layer_set_text_color(order_submitted_text_layer, GColorBlack);
  text_layer_set_background_color(order_submitted_text_layer, GColorWhite);
  text_layer_set_text(order_submitted_text_layer, order_total_buffer);
  layer_add_child(window_layer, text_layer_get_layer(order_submitted_text_layer));
}

static void order_submitted_unload(Window *widnow) {
  text_layer_destroy(order_submitted_text_layer);
}

// *** END SUBMITTED WINDOW FUNCTIONS ***

//------------------------------------

static void init() {

  if (persist_exists(KEY_BALANCE_DOLLARS) && persist_exists(KEY_BALANCE_CENTS)) {

    balance_dollars = persist_read_int(KEY_BALANCE_DOLLARS); 
    balance_cents = persist_read_int(KEY_BALANCE_CENTS); 

    balance_total = balance_dollars;
    temp = balance_cents;
    balance_total = balance_total + (temp/100);
  } 

  if (persist_exists(KEY_LETTER_1))
  {
    letter_count_1 = persist_read_int(KEY_LETTER_1);
  }  else letter_count_1 = 0;
  if (persist_exists(KEY_LETTER_2))
  {
    letter_count_2 = persist_read_int(KEY_LETTER_2);
  }  else letter_count_2 = 0;
  if (persist_exists(KEY_LETTER_3))
  {
    letter_count_3 = persist_read_int(KEY_LETTER_3);
  }  else letter_count_3 = 0;

  persist_read_string(KEY_NAME_STORE, name, 3);



  main_window = window_create();
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });

  map_window = window_create();
  window_set_window_handlers(map_window, (WindowHandlers) {
    .load = map_window_load,
    .unload = map_window_unload,
  });

  balance_window = window_create();
  window_set_window_handlers(balance_window, (WindowHandlers) {
    .load = balance_window_load,
    .unload = balance_window_unload,
  });

  order_window = window_create();
  window_set_window_handlers(order_window, (WindowHandlers) {
    .load = order_window_load,
    .unload = order_window_unload,
  });

  confirmation_window = window_create();
  window_set_window_handlers(confirmation_window, (WindowHandlers) {
    .load = confirmation_window_load,
    .unload = confirmation_window_unload,
  });

  order_submitted_window = window_create();
  window_set_window_handlers(order_submitted_window, (WindowHandlers) {
    .load = order_submitted_load,
    .unload = order_submitted_unload,
  });

  drink_specifics_window = window_create();
  window_set_window_handlers(drink_specifics_window, (WindowHandlers) {
    .load = drink_window_load,
    .unload = drink_window_unload,
  });

  name_window = window_create();
  window_set_window_handlers(name_window, (WindowHandlers) {
    .load = name_window_load,
    .unload = name_window_unload,
  });

  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  window_stack_push(main_window, true);  //whatever is pushed here will be shown on app start
}

static void deinit() {
  window_destroy(main_window);
  window_destroy(map_window);
  window_destroy(balance_window);
  window_destroy(order_window);
  window_destroy(confirmation_window);
  window_destroy(order_submitted_window);
  window_destroy(drink_specifics_window);
  window_destroy(name_window);

  balance_dollars = balance_total;
  balance_cents = (balance_total * 100) - (balance_dollars * 100);

  persist_write_int(KEY_BALANCE_DOLLARS, (balance_dollars));
  persist_write_int(KEY_BALANCE_CENTS, (balance_cents));
  persist_write_int(KEY_LETTER_1, letter_count_1);
  persist_write_int(KEY_LETTER_2, letter_count_2);
  persist_write_int(KEY_LETTER_3, letter_count_3);
  persist_write_string(KEY_NAME_STORE, name);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}