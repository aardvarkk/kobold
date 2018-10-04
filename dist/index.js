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
var pgp = require('pg-promise')();
var db = pgp(process.env.DATABASE_URL || "postgresql://localhost/kosi");
var app = express_1.default();
app.use(body_parser_1.default.json());
var recordTemperature = function (userDeviceId, temperature) {
    return __awaiter(this, void 0, void 0, function () {
        return __generator(this, function (_a) {
            db.none("\n    INSERT INTO reports (user_device_id, temperature)\n    VALUES ($1, $2)\n  ", [userDeviceId, temperature]);
            return [2 /*return*/];
        });
    });
};
var deviceInstruction = function (temperature) {
    return __awaiter(this, void 0, void 0, function () {
        return __generator(this, function (_a) {
            if (temperature < 18.0) {
                return [2 /*return*/, 201];
            }
            else {
                return [2 /*return*/, 200];
            }
            return [2 /*return*/];
        });
    });
};
app.post('/', function (req, res) {
    return __awaiter(this, void 0, void 0, function () {
        var auth, token, userDevice, code;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("Received report");
                    console.log(req.body);
                    auth = (req.get("Authorization") || "").split(' ');
                    token = auth[1];
                    return [4 /*yield*/, db.oneOrNone("\n    SELECT id\n    FROM user_devices\n    WHERE token = $1\n  ", [token])];
                case 1:
                    userDevice = _a.sent();
                    if (userDevice === null) {
                        console.log("Bad token " + token);
                        res.status(401).end();
                        return [2 /*return*/];
                    }
                    if (!req.body.temperature) {
                        console.log('Bad report');
                        res.status(400).end();
                        return [2 /*return*/];
                    }
                    // Asynchronously record the temperature
                    recordTemperature(userDevice.id, req.body.temperature);
                    return [4 /*yield*/, deviceInstruction(req.body.temperature)];
                case 2:
                    code = _a.sent();
                    res.status(code).end();
                    return [2 /*return*/];
            }
        });
    });
});
// NOTE: Cascades into reports
var destroyDevice = function (deviceId) {
    return __awaiter(this, void 0, void 0, function () {
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0: return [4 /*yield*/, db.none("\n    DELETE FROM user_devices\n    WHERE device_id = $1\n  ", [deviceId])];
                case 1:
                    _a.sent();
                    return [2 /*return*/];
            }
        });
    });
};
// Links a device to a user account and gives back a token to use to send data
var linkUserDevice = function (userId, deviceId) {
    return __awaiter(this, void 0, void 0, function () {
        var userDevice;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("linkUserDevice");
                    // Destroy any link the device has to other users
                    return [4 /*yield*/, destroyDevice(deviceId)
                        // Create the new link
                    ];
                case 1:
                    // Destroy any link the device has to other users
                    _a.sent();
                    return [4 /*yield*/, db.one("\n    INSERT INTO user_devices (user_id, device_id, token)\n    VALUES (\n      $1,\n      $2,\n      encode(digest(concat(now(), random()), 'sha256'), 'hex')\n    )\n    RETURNING token\n  ", [userId, deviceId])];
                case 2:
                    userDevice = _a.sent();
                    return [2 /*return*/, userDevice.token];
            }
        });
    });
};
// Links a device, which obtains a token and starts reporting into a given user's account
// Requires four parameters:
// Device Key, Device Secret
// Email, Password
// If they're all correct, we will create a user_device token and return it
// TODO: 400 on bad key/secret
// TODO: 401 on bad email/password
app.post('/link', function (req, res) {
    return __awaiter(this, void 0, void 0, function () {
        var device, user, token;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("Link request");
                    console.log(req.body.key);
                    console.log(req.body.secret);
                    console.log(req.body.email);
                    console.log(req.body.password);
                    return [4 /*yield*/, db.oneOrNone("\n    SELECT id\n    FROM devices\n    WHERE key = $1\n    AND secret = crypt($2, secret)\n  ", [req.body.key, req.body.secret])];
                case 1:
                    device = _a.sent();
                    console.log(device);
                    return [4 /*yield*/, db.oneOrNone("\n    SELECT id\n    FROM users\n    WHERE email = $1\n    AND password = crypt($2, password)\n  ", [req.body.email, req.body.password])];
                case 2:
                    user = _a.sent();
                    console.log(user);
                    return [4 /*yield*/, linkUserDevice(user.id, device.id)];
                case 3:
                    token = _a.sent();
                    res.send({ "token": token });
                    return [2 /*return*/];
            }
        });
    });
});
var port = parseInt(process.env.PORT || "3000");
app.listen(port, function () {
    console.log("Listening at http://localhost:" + port + "/");
});
