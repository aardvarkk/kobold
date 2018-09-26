import * as express from 'express'
const app: express.Application = express.default()

app.post('/', function(req: express.Request, res: express.Response) {
  // TODO: Determine if/when to send reset response
  res.status(205);
})

const port: number = parseInt(process.env.PORT || "3000")

app.listen(port, () => {
    console.log(`Listening at http://localhost:${port}/`)
});