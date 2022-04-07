// preload.js
const net = require("net");

const HOST = "192.168.0.1";
const PORT = 5000;

// establish connection to the server
const client = new net.Socket();
try{
  client.connect({port: PORT, host: HOST});
}
catch (error){
  console.log("Cannot find server...");
}


let userAction = {
  "action": "POST"
};

let serverRequest = {
  "action": "DUMP"
};



// All of the Node.js APIs are available in the preload process.
// It has the same sandbox as a Chrome extension.
window.addEventListener("load", () => {
  let startGameBtn = document.getElementById("startGameBtn");
  let endGameBtn = document.getElementById("endGameBtn");
  let devModal = document.getElementById("devModal");
  let osuLogo = document.getElementById("osuLogo");
  let devModalCloseBtn = document.getElementById("devModalCloseBtn");
  startGameBtn.addEventListener("click", () => {
    userAction["game_flag"] = "1";
    client.write("empt"+JSON.stringify(userAction));
    console.log(userAction);
  });
  endGameBtn.addEventListener("click", () => {
    userAction["game_flag"] = "0";
    client.write("empt"+JSON.stringify(userAction));
    console.log(userAction);
  });
  
  // TODO Modal Buttons + Event Listeners
  
  osuLogo.addEventListener("click", () => {
    devModal.style.display = "block";
  });
  
  devModalCloseBtn.addEventListener("click", () => {
    devModal.style.display = "none";
  });
  
});



// periodically poll the server for data
setInterval(() => {
  let devModalContentText = document.getElementById("devModalContentText");
  let playerScore = document.getElementById("playerScore");
  let robotScore = document.getElementById("robotScore");
  // console.log(JSON.stringify(serverRequest));
  if (client.write("empt"+JSON.stringify(serverRequest))){
    client.on('data', (data) => {
      let json_data = JSON.parse(data);
      // console.log(JSON.stringify(json_data, null, 4)); 
      devModalContentText.innerHTML = JSON.stringify(json_data, undefined, 4);
      if("player_score" in data){
        playerScore.innerHTML = data["player_score"];
      }
      if("robot_score" in data){
        robotScoreinnerHTML = data["robot_score"];
      }
    });
  }
  else {
    console.log("Error sending message...");
    client.connect({port: PORT, host: HOST});
  }
}, 800);
