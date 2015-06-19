/*
 * pebble-js-app.js
 * Sends the sample message once it is initialized.
 */

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS Ready!');
});

var request = new XMLHttpRequest();
request.open('POST', 'https://mandrillapp.com/api/1.0/messages/send.json', true);
request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded; charset=UTF-8');

Pebble.addEventListener('appmessage', function(e) {

  console.log('Received appmessage: ' + JSON.stringify(e.payload));

  var order_total = JSON.stringify(e.payload["KEY_ORDER_TOTAL"]);

  var name = JSON.stringify(e.payload["KEY_NAME"]);

  var item_1 = JSON.stringify(e.payload["KEY_ITEM_1"]);
  var item_2 = JSON.stringify(e.payload["KEY_ITEM_2"]);
  var item_3 = JSON.stringify(e.payload["KEY_ITEM_3"]);
  var item_4 = JSON.stringify(e.payload["KEY_ITEM_4"]);
  var item_5 = JSON.stringify(e.payload["KEY_ITEM_5"]);

  var item_size_1 = JSON.stringify(e.payload["KEY_ITEM_SIZE_1"]);
  var item_size_2 = JSON.stringify(e.payload["KEY_ITEM_SIZE_2"]);
  var item_size_3 = JSON.stringify(e.payload["KEY_ITEM_SIZE_3"]);
  var item_size_4 = JSON.stringify(e.payload["KEY_ITEM_SIZE_4"]);
  var item_size_5 = JSON.stringify(e.payload["KEY_ITEM_SIZE_5"]);

  if (!order_total){order_total = "";}
  if (!name){name = "";}
  if (!item_1){item_1 = "";}
  if (!item_2){item_2 = "";}
  if (!item_3){item_3 = "";}
  if (!item_4){item_4 = "";}
  if (!item_5){item_5 = "";}
  if (!item_size_1){item_size_1 = "";}
  if (!item_size_2){item_size_2 = "";}
  if (!item_size_3){item_size_3 = "";}
  if (!item_size_4){item_size_4 = "";}
  if (!item_size_5){item_size_5 = "";}

  order_total = order_total.replace(/["']/g, "");
  name = name.replace(/["']/g, "");
  item_1 = item_1.replace(/["']/g, ""); 
  item_2 = item_2.replace(/["']/g, ""); 
  item_3 = item_3.replace(/["']/g, ""); 
  item_4 = item_4.replace(/["']/g, ""); 
  item_5 = item_5.replace(/["']/g, ""); 
  item_size_1 = item_size_1.replace(/["']/g, "");
  item_size_2 = item_size_2.replace(/["']/g, "");  
  item_size_3 = item_size_3.replace(/["']/g, ""); 
  item_size_4 = item_size_4.replace(/["']/g, ""); 
  item_size_5 = item_size_5.replace(/["']/g, "");
  
  console.log(" stringified 0 (Order total): " + order_total);
  console.log(" stringified 1 (Name): " + name);
  console.log(" stringified 2 (Item 1): " + item_1);
  console.log(" stringified 3 (Item 2): " + item_2);
  console.log(" stringified 4 (Item 3): " + item_3);
  console.log(" stringified 5 (Item 4): " + item_4);
  console.log(" stringified 6 (Item 5): " + item_5);
  console.log(" stringified 7 (Item 1 Size): " + item_size_1);
  console.log(" stringified 8 (Item 2 Size): " + item_size_2);
  console.log(" stringified 9 (Item 3 Size): " + item_size_3);
  console.log(" stringified 10 (Item 4 Size): " + item_size_4);
  console.log(" stringified 11 (Item 5 Size): " + item_size_5);

  request.send(JSON.stringify({
    "message": {
      "from_email": "poolec@gwu.edu",
      "to": [
        {
          "email": "poolec@gwu.edu"
        },
        {
          "email": "5184958434@txt.att.net"
        }
      ],
      "subject": "New BariBucks order!",
      //  **BASIC TEXT FIELD BESIDES HTML BELOW** "text": "\nName on order: " + name + "\n\nItem 1: " + item_size_1 + item_1 + "\nItem 2: " + item_size_2 + item_2 + "\nItem 3: "  + item_size_3 + item_3 + "\nItem 4: " + item_size_4 + item_4 + "\nItem 5: " + item_size_5 +item_5
      "html": "<!DOCTYPE html><html><head><meta content=\"text/html; charset=windows-1252\" http-equiv=\"content-type\"><title></title></head><body class=\"New\" style=\" width: 800px;\"><div style=\"text-align: center;\"><a href=\"http://i.imgur.com/5U29iF5.png\"><img style=\"height: 170px; width: 625px;\"src=\"http://i.imgur.com/5U29iF5.png\"title=\"source: http://i.imgur.com/5U29iF5.png\"></a></div><br><h1> A new order has been submitted!</h1><h4>Name on order: "+ name + "</h4><h4>Order Toal: "+ order_total + "</h4> <p><span>Item 1: " + item_size_1 + " " + item_1 +"</span></p><p>Item 2: "+ item_size_2 + " " +  item_2 +"</p><p>Item 3: "+ item_size_3 + " " +  item_3 +"</p><p>Item 4: "+item_size_4 + " " +  item_4 +"</p><p>Item 5: "+item_size_5 + " " +  item_5 +"</p><br></body></html>"   
    },
    "async": false,
    "ip_pool": null,
    "send_at": null,
    "key": "jjSIJe9D1AP4g6Sj3raPLw"

  })
              ); 
  console.log('Sent XML request');

});
