const express = require('express');
const path = require('path');
const app = express()
const http = require('http');
const mqtt = require("mqtt");
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server);
const port = process.env.PORT || 3000;

const clientId = 'mqttjs_' + Math.random().toString(16).substr(2, 8)
const host = 'mqtt://broker.hivemq.com'


let SOCKET_CLIENT;


app.use(express.json());

app.get('/', (req, res) => {
  res.sendFile(__dirname + '/index.html');
});
app.use('/folder', express.static(path.join(__dirname, 'public2')))
app.use('/about', express.static(path.join(__dirname, 'about')))
app.use('/more', express.static(path.join(__dirname, 'more')))
app.use('/contacts', express.static(path.join(__dirname, 'contacts')))

app.post('/button', (req, res) => {
  value = req.body
  console.log(req.body)
   //html css js
if(req.body.button_one_value === true){
  res.send({
      "status": true,
      "messege": "mqtt button1 is clicked"
    })
  let sendMessage = "P";
  client.publish('sonu', sendMessage, { qos: 0, retain: false })
  console.log("Button1");
}
else if(req.body.button_one_value === false){
  res.send({
      "status": true,
      "messege": "mqtt button2 is clicked"
    })
  let sendMessage = "Q";
  client.publish('sonu', sendMessage, { qos: 0, retain: false }) 
}

else if(req.body.button_two_value === true){
res.send({
    "status": true,
    "messege": "mqtt button2 is clicked"
  })
let sendMessage = "R";
client.publish('sonu', sendMessage, { qos: 0, retain: false }) 
}

else if(req.body.button_two_value === false){
res.send({
    "status": true,
    "messege": "mqtt button2 is clicked"
  })
let sendMessage = "S";
client.publish('sonu', sendMessage, { qos: 0, retain: false }) 
}

else if(req.body.button_three_value === true){
res.send({
    "status": true,
    "messege": "mqtt button2 is clicked"
  })
let sendMessage = "T";
client.publish('sonu', sendMessage, { qos: 0, retain: false }) 
}

else if(req.body.button_three_value === false){
res.send({
    "status": true,
    "messege": "mqtt button2 is clicked"
  })
let sendMessage = "U";
client.publish('sonu', sendMessage, { qos: 0, retain: false }) 
}

else if(req.body.button_four_value === true){
res.send({
    "status": true,
    "messege": "mqtt button2 is clicked"
  })
let sendMessage = "V";
client.publish('sonu', sendMessage, { qos: 0, retain: false }) 
}


else if(req.body.button_four_value === false){
res.send({
    "status": true,
    "messege": "mqtt button2 is clicked"
  })
let sendMessage = "W";
client.publish('sonu', sendMessage, { qos: 0, retain: false }) 
}

else{
console.log("O");
}






  
  
})

server.listen(port, () => {
    console.log(`Example app listening on port ${port}`)
})


const options = {
    keepalive: 30,
    clientId: clientId,
    protocolId: 'MQTT',
    protocolVersion: 4,
    clean: true,
    reconnectPeriod: 1000,
    connectTimeout: 30 * 1000,
    will: {
      topic: 'WillMsg',
      payload: 'Connection Closed abnormally..!',
      qos: 0,
      retain: false
    },
    rejectUnauthorized: false
  }
  
  console.log('connecting mqtt client')
  const client = mqtt.connect(host, options)


  client.on('error', function (err) {
  console.log(err)
  client.end()
})


client.on('connect', function () {
      console.log('client connected:' + clientId)
      client.subscribe('esp32_mqtt_tempvalue', { qos: 0 },(sub_message)=>{
          console.log("Message From Server : "+sub_message); 
        }) 
        
      client.subscribe('esp32_mqtt_temp_humidvalue', { qos: 0 },(sub_message)=>{
            console.log("Message From Server : "+sub_message); 
        })  
    })



    



try {
  
    io.on("connection", (socket) => {
      SOCKET_CLIENT =  socket;
      console.log(`User with id: ${socket.id} connected!`);
  
      socket.on("disconnect", () => {
        console.log(`User with id: ${socket.id} disconnected`);
      });
    
  
      // io.emit("Chart-Data",message)
      socket.on("hello_server", (data) => {
        console.log(data);
        let senddata = "temp";
        
        socket.emit("weather", senddata);
      });
      client.on('message', function (topic, message, packet) {
        // console.log('Received Message:= ' + message.toString() + '\nOn topic:= ' + topic)
        // console.log("\n"+JSON.stringify(packet))
       if(topic === "esp32_mqtt_tempvalue"){
          SOCKET_CLIENT.emit("weather", message.toString());
       }
       else if(topic === "esp32_mqtt_temp_humidvalue"){
          
          SOCKET_CLIENT.emit("hum", message.toString());
       }
        
      })
   
  
    });
  
    //admin-ui
    // instrument(io, { auth: false });
  } catch (error) {
    console.log(`Could not start the server, ${error}`);
  }