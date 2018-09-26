import * as express from 'express'
import * as bodyParser from 'body-parser'

const app: express.Application = express.default()

app.use(bodyParser.json());

app.post('/', function(req: express.Request, res: express.Response) {
  // TODO: Determine if/when to send reset response
  console.log("Received report");
  console.log(req.body);
  res.status(205).end();
})

const port: number = parseInt(process.env.PORT || "3000")

app.listen(port, () => {
    console.log(`Listening at http://localhost:${port}/`)
});
