"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : new P(function (resolve) { resolve(result.value); }).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var express_1 = __importDefault(require("express"));
var body_parser_1 = __importDefault(require("body-parser"));
var basic_auth_1 = __importDefault(require("basic-auth"));
var pgp = require('pg-promise')();
var db = pgp(process.env.DATABASE_URL || "postgresql://localhost/kosi");
var app = express_1.default();
app.use(body_parser_1.default.json());
app.post('/', function (req, res) {
    console.log("Received report");
    console.log("Auth: " + req.get("Authorization"));
    console.log(req.body);
    // TODO: Return 200 if cool
    // TODO: Return 201 if heat
    // TODO: Return 401 if bad token
    // TODO: Return 403 if unlinked
    // TODO: Return 405 if reset requested
    res.status(401).end();
});
var grantDeviceToken = function (deviceId) {
    return __awaiter(this, void 0, void 0, function () {
        var data, e_1;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("grantDeviceToken");
                    _a.label = 1;
                case 1:
                    _a.trys.push([1, 3, , 4]);
                    return [4 /*yield*/, db.oneOrNone("\n      UPDATE devices \n      SET token = encode(digest(concat(now(), random()), 'sha256'), 'hex')\n      WHERE id = $1\n      RETURNING token\n    ", [deviceId])];
                case 2:
                    data = _a.sent();
                    if (data !== null) {
                        return [2 /*return*/, data.token];
                    }
                    return [3 /*break*/, 4];
                case 3:
                    e_1 = _a.sent();
                    console.log(e_1);
                    return [3 /*break*/, 4];
                case 4: return [2 /*return*/, null];
            }
        });
    });
};
// Device token allows a device to:
// 1. Report quickly without having to hash secrets
// 2. Reauthenticate if some strange action happens on the account
app.post('/token', function (req, res) {
    return __awaiter(this, void 0, void 0, function () {
        var auth, data, token, e_2;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("Device token request");
                    auth = basic_auth_1.default(req);
                    if (!auth) return [3 /*break*/, 8];
                    console.log("Device Key: " + auth.name + ", Secret: " + auth.pass);
                    _a.label = 1;
                case 1:
                    _a.trys.push([1, 6, , 7]);
                    return [4 /*yield*/, db.oneOrNone("\n        SELECT id\n        FROM devices\n        WHERE key = $1\n        AND secret = crypt($2, secret)\n        LIMIT 1\n      ", [auth.name, auth.pass])];
                case 2:
                    data = _a.sent();
                    if (!(data !== null)) return [3 /*break*/, 4];
                    console.log("Successful auth");
                    return [4 /*yield*/, grantDeviceToken(data.id)];
                case 3:
                    token = _a.sent();
                    res.send(token);
                    return [3 /*break*/, 5];
                case 4:
                    console.log("Unsuccessful auth");
                    res.status(400).end();
                    _a.label = 5;
                case 5: return [3 /*break*/, 7];
                case 6:
                    e_2 = _a.sent();
                    console.log(e_2);
                    return [3 /*break*/, 7];
                case 7: return [3 /*break*/, 9];
                case 8:
                    console.log("Malformed auth");
                    res.status(400).end();
                    _a.label = 9;
                case 9: return [2 /*return*/];
            }
        });
    });
});
// Linking a device means it starts reporting into a given user's account
app.post('/link', function (req, res) {
    // Search for key
    // Check that hashed secret matches prepopulated DB
    var a = "\n  SELECT EXISTS(\n    SELECT 1\n    FROM devices\n    WHERE key = 'D82V8IDgiJUgPwj9ZbbXcS3r002kgiUX' \n    AND secret = crypt('NEqaSnzcX-rWRFGhZXoFEro8e-EwGK8J', secret)\n    LIMIT 1\n  )\n  ";
    // If matched, add user-device link
    // Require user account info from user token
    var b = "\n  INSERT INTO user_devices (user_id, device_id) VALUES (1, 1)\n  ";
    // Clear all device data obtained from other user links
    var c = "\n  DELETE FROM reports WHERE user_device_id != 123\n  ";
});
var port = parseInt(process.env.PORT || "3000");
app.listen(port, function () {
    console.log("Listening at http://localhost:" + port + "/");
});
