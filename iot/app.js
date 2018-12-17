const express = require("express");
const config = require("./config.js");
const path = require('path');
const port = 3000;
const app = express();

app.use(express.static(path.join(__dirname, 'node_modules')));
app.use(express.static(path.join(__dirname, 'static')));

app.get('/', (req, res) => {
    res.sendFile("index.html", {
        root: __dirname + '/static'
    });
});

app.get('/status', (req, res) => {
    const date = new Date();
    res.setHeader('Content-type', 'application/json');
    res.send(
        JSON.stringify({
            status: config.status,
            detected: (date.getTime() - config.detected <= 1000)
        })
    );
});

app.get('/detected', (req, res) => {
    const date = new Date();
    config.detected = date.getTime();
    console.log("Detect an object at " + date.toLocaleString());
    res.send("OK");
});

app.get('/door', (req, res) => {
    res.send(config.status);
});

app.get('/open', (req, res) => {
    console.log("open door");
    config.status = 1;
    res.send("OK");
});

app.get('/close', (req, res) => {
    console.log("close door");
    config.status = 0;
    res.send("OK");
});

app.listen(port, () => {
    console.log(`The server is running on port ${port}!`)
});