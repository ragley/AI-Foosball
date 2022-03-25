// preload.js
const net = require("net");

const HOST = "192.168.0.1";
const PORT = 5000;

let serverRequest = {
  "action": "DUMP"
};

// All of the Node.js APIs are available in the preload process.
// It has the same sandbox as a Chrome extension.
window.addEventListener("load", () => {
  document.getElementById("startGameBtn").addEventListener("click", () => {
    console.log("start game button");
  });
  document.getElementById("endGameBtn").addEventListener("click", () => {
    console.log("end game button");
  });
});

const client = new net.Socket();
client.connect({port: PORT, host: HOST});
setInterval(() => {
  console.log(JSON.stringify(serverRequest));
  if (client.write("empt"+JSON.stringify(serverRequest))){
    client.on('data', (data) => {
      console.log(data.toString('utf-8'));
      // update elements of the DOM here.
    });
  }
  else {
    console.log("Did not send all data...");
  }
}, 500);
