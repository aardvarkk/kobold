import express from 'express'
import bodyParser from 'body-parser'
import basicAuth from 'basic-auth'

const pgp = require('pg-promise')()
const db = pgp(process.env.DATABASE_URL || "postgresql://localhost/kosi")

const app: express.Application = express()

app.use(bodyParser.json())

app.post('/', function(req: express.Request, res: express.Response) {
  console.log("Received report")
  console.log(`Auth: ${req.get("Authorization")}`)
  console.log(req.body)

  // TODO: Return 200 if cool
  // TODO: Return 201 if heat
  // TODO: Return 401 if bad token
  // TODO: Return 403 if unlinked
  // TODO: Return 405 if reset requested

  res.status(401).end()
})

const grantDeviceToken = async function(deviceId: string) {
  console.log("grantDeviceToken")

  try {
    const data = await db.oneOrNone(`
      UPDATE devices 
      SET token = encode(digest(concat(now(), random()), 'sha256'), 'hex')
      WHERE id = $1
      RETURNING token
    `, [deviceId])

    if (data !== null) {
      return data.token
    }
  } catch(e) {
    console.log(e)
  }

  return null
}

// Device token allows a device to:
// 1. Report quickly without having to hash secrets
// 2. Reauthenticate if some strange action happens on the account
app.post('/token', async function(req: express.Request, res: express.Response) {
  console.log("Device token request")
  const auth = basicAuth(req)

  if (auth) {
    console.log(`Device Key: ${auth.name}, Secret: ${auth.pass}`)

    try {
      const data = await db.oneOrNone(`
        SELECT id
        FROM devices
        WHERE key = $1
        AND secret = crypt($2, secret)
        LIMIT 1
      `, [auth.name, auth.pass])

      if (data !== null) {
        console.log("Successful auth")
        const token = await grantDeviceToken(data.id)
        res.send(token)
      } else {
        console.log("Unsuccessful auth")
        res.status(400).end()
      }
    } catch(e) {
      console.log(e)
    }
  } else {
    console.log("Malformed auth")
    res.status(400).end()
  }
})

// Linking a device means it starts reporting into a given user's account
app.post('/link', function(req: express.Request, res: express.Response) {
  // Search for key
  // Check that hashed secret matches prepopulated DB
  
  const a = `
  SELECT EXISTS(
    SELECT 1
    FROM devices
    WHERE key = 'D82V8IDgiJUgPwj9ZbbXcS3r002kgiUX' 
    AND secret = crypt('NEqaSnzcX-rWRFGhZXoFEro8e-EwGK8J', secret)
    LIMIT 1
  )
  `

  // If matched, add user-device link
  // Require user account info from user token
  const b = `
  INSERT INTO user_devices (user_id, device_id) VALUES (1, 1)
  `
  
  // Clear all device data obtained from other user links
  const c = `
  DELETE FROM reports WHERE user_device_id != 123
  `
})

const port: number = parseInt(process.env.PORT || "3000")

app.listen(port, () => {
  console.log(`Listening at http://localhost:${port}/`)
})
