const { app, BrowserWindow, ipcMain } = require("electron");
const AutoLaunch = require('auto-launch');
const path = require("path");

// create and display window ===========================================
// load the respective html
const createWindow = () => {
  const mainWindow = new BrowserWindow({
    fullscreen: true,
    webPreferences: {
      preload: path.join(__dirname, "./src/preload.js"),
    },
  });
  // mainWindow.setMenu(null);
  mainWindow.loadFile("./src/index.html");
};

app.whenReady().then(() => {
  createWindow();

  app.on("activate", () => {
    if (BrowserWindow.getAllWindows().length === 0) createWindow();
    // Set autolaunch ==================================================
    let autoLauncher = new AutoLaunch({
      name: "Foosbots-GUI"
    });

    autoLauncher.isEnabled().then((isEnabled) => {
      if(isEnabled) return;
      console.log("Autolauncher enabled :)");
      autoLauncher.enable();
    }).catch((err) => {
      throw err;
    });
    // -----------------------------------------------------------------
  });
});


app.on("window-all-closed", () => {
  if (process.platform !== "darwin") app.quit();
});
// ---------------------------------------------------------------------

// connecting to the server ============================================
//const net = require("net");

//const HOST = "192.168.0.1";
//const PORT = 5000;

//let serverRequest = {
  //"action": "DUMP"
//};

//window.addEventListener("load", () => {
  //document.getElementById("startGameBtn").addEventListener("click", () => {
    //console.log("start game button");
  //});
  //document.getElementById("endGameBtn").addEventListener("click", () => {
    //console.log("end game button");
  //});
//});

//window.setTimeout(() => {console.log("Hello...")}, 50);

//const client = new net.Socket();
//client.connect({port: PORT, host: HOST});
//setTimeout(() => {
  //console.log(JSON.stringify(serverRequest));
  //if (client.write("empt"+JSON.stringify(serverRequest))){
    //client.on('data', (data) => {
      //// console.log(data.toString('utf-8'));
      //// update elements of the DOM here.
      
    //});
  //}
  //else {
    //console.log("Did not send all data...");
  //}
//}, 50);


