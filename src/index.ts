import express from 'express'
import bodyParser from 'body-parser'

const pgp = require('pg-promise')()
const db = pgp(process.env.DATABASE_URL || "postgresql://localhost/kosi")

const app: express.Application = express()

app.use(bodyParser.json())

const recordTemperature = async function(userDeviceId: number, temperature: number) {
  db.none(`
    INSERT INTO reports (user_device_id, temperature)
    VALUES ($1, $2)
  `, [userDeviceId, temperature])
}

const deviceInstruction = async function(temperature: number) {
  if (temperature < 18.0) {
    return 201;
  } else {
    return 200;
  }
}

app.post('/', async function(req: express.Request, res: express.Response) {
  console.log("Received report")
  console.log(req.body)

  const auth = (req.get("Authorization") || "").split(' ')
  const token = auth[1]
  const userDevice = await db.oneOrNone(`
    SELECT id
    FROM user_devices
    WHERE token = $1
  `, [token])

  if (userDevice === null) {
    console.log(`Bad token ${token}`)
    res.status(401).end();
    return;
  }

  if (!req.body.temperature) {
    console.log('Bad report')
    res.status(400).end();
    return;
  }

  // Asynchronously record the temperature
  recordTemperature(userDevice.id, req.body.temperature)

  // TODO: Return 405 if reset requested

  const code = await deviceInstruction(req.body.temperature)
  
  res.status(code).end()
})

// NOTE: Cascades into reports
const destroyDevice = async function(deviceId: number) {
  await db.none(`
    DELETE FROM user_devices
    WHERE device_id = $1
  `, [deviceId])
}

// Links a device to a user account and gives back a token to use to send data
const linkUserDevice = async function(userId: number, deviceId: number) {
  console.log("linkUserDevice")

  // Destroy any link the device has to other users
  await destroyDevice(deviceId)

  // Create the new link
  const userDevice = await db.one(`
    INSERT INTO user_devices (user_id, device_id, token)
    VALUES (
      $1,
      $2,
      encode(digest(concat(now(), random()), 'sha256'), 'hex')
    )
    RETURNING token
  `, [userId, deviceId])

  return userDevice.token
}

// Links a device, which obtains a token and starts reporting into a given user's account
// Requires four parameters:
// Device Key, Device Secret
// Email, Password
// If they're all correct, we will create a user_device token and return it
// TODO: 400 on bad key/secret
// TODO: 401 on bad email/password
app.post('/link', async function(req: express.Request, res: express.Response) {
  console.log("Link request")
  console.log(req.body.key)
  console.log(req.body.secret)
  console.log(req.body.email)
  console.log(req.body.password)

  const device = await db.oneOrNone(`
    SELECT id
    FROM devices
    WHERE key = $1
    AND secret = crypt($2, secret)
  `, [req.body.key, req.body.secret])

  console.log(device)

  const user = await db.oneOrNone(`
    SELECT id
    FROM users
    WHERE email = $1
    AND password = crypt($2, password)
  `, [req.body.email, req.body.password])

  console.log(user)

  const token = await linkUserDevice(user.id, device.id)
  res.send({ "token": token })
})

const port: number = parseInt(process.env.PORT || "3000")

app.listen(port, () => {
  console.log(`Listening at http://localhost:${port}/`)
})
