"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var express_1 = __importDefault(require("express"));
var body_parser_1 = __importDefault(require("body-parser"));
var basic_auth_1 = __importDefault(require("basic-auth"));
var app = express_1.default();
app.use(body_parser_1.default.json());
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
    var auth = basic_auth_1.default(req);
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
