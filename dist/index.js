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
var app = express.default();
app.post('/', function (req, res) {
    // TODO: Determine if/when to send reset response
    res.status(205);
});
var port = parseInt(process.env.PORT || "3000");
app.listen(port, function () {
    console.log("Listening at http://localhost:" + port + "/");
});
