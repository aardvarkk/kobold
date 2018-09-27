"use strict";
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (Object.hasOwnProperty.call(mod, k)) result[k] = mod[k];
    result["default"] = mod;
    return result;
};
Object.defineProperty(exports, "__esModule", { value: true });
var express = __importStar(require("express"));
var bodyParser = __importStar(require("body-parser"));
var basicAuth = __importStar(require("basic-auth"));
var app = express.default();
app.use(bodyParser.json());
app.post('/', function (req, res) {
    // TODO: Determine if/when to send reset response
    console.log("Received report");
    console.log("Auth: " + req.get("Authorization"));
    console.log(req.body);
    // TODO: Return 200 if token is valid
    res.status(401).end();
});
app.post('/token', function (req, res) {
    console.log("Token request");
    var auth = basicAuth.parse(req.get("Authorization") || "");
    if (auth) {
        console.log("Username: " + auth.name + ", Password: " + auth.pass);
        res.send("abc123");
    }
    else {
        res.status(400).end();
    }
});
var port = parseInt(process.env.PORT || "3000");
app.listen(port, function () {
    console.log("Listening at http://localhost:" + port + "/");
});
