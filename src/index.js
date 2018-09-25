"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var http = require("http");
var port = 3000;
var requestHandler = function (request, response) {
    console.log(request.url);
    response.end('Hello Node.js Server!');
};
var server = http.createServer(requestHandler);
server.listen(port, function (err) {
    if (err) {
        return console.log('something bad happened', err);
    }
    console.log("server is listening on " + port);
});
