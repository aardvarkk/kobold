import express from 'express'
import bodyParser from 'body-parser'
import basicAuth from 'basic-auth'

const app: express.Application = express()

app.use(bodyParser.json());

app.post('/', function(req: express.Request, res: express.Response) {
  // TODO: Determine if/when to send reset response
  console.log("Received report");
  console.log(`Auth: ${req.get("Authorization")}`)
  console.log(req.body)

  // TODO: Return 200 if token is valid
  res.status(401).end()
})

app.post('/token', function(req: express.Request, res: express.Response) {
  console.log("Token request")
  const auth = basicAuth(req)
  if (auth) {
    console.log(`Username: ${auth.name}, Password: ${auth.pass}`)
    res.send("abc123")
  } else {
    res.status(400).end()
  }
})

const port: number = parseInt(process.env.PORT || "3000")

app.listen(port, () => {
    console.log(`Listening at http://localhost:${port}/`)
});
