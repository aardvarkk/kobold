var express = require('express')
var app = express()
var bodyParser = require('body-parser')
var passport = require('passport')
var LocalStrategy = require('passport-local').Strategy

const Sequelize = require('sequelize');
const sequelize = new Sequelize('kobold', 'aardvarkk', null, {
  dialect: 'postgres'
});

const Influx = require('influx')
const influx = new Influx.InfluxDB({
	host: 'localhost',
	database: 'kobold'
})

app.set('view engine', 'ejs')
app.use(bodyParser.json())
app.use(bodyParser.urlencoded())

passport.use(new LocalStrategy(
  function(username, password, done) {
    User.findOne({ username: username }, function (err, user) {
      if (err) { return done(err) }
      if (!user) {
        return done(null, false, { message: 'Incorrect username.' })
      }
      if (!user.validPassword(password)) {
        return done(null, false, { message: 'Incorrect password.' })
      }
      return done(null, user)
    })
  }
))

app.get('/login', function(req, res) {
	res.render('login.ejs')
})

app.post('/login', passport.authenticate('local'), function(req, res) {
  res.redirect('/users/' + req.user.username)
})

app.get('/', function(req, res) {
	influx.query('SELECT * FROM readings').then(result => {
		res.render('index.ejs', {
			data: result
		})
	}).catch(err => {
		res.status(500).send(err.stack)
	})
})

app.post('/', function(req, res) {
	console.log(req.body)

	influx.writePoints([
	{
		measurement: 'readings',
		tags: { sensor: req.body.sensor },
		fields: {
			temperature: req.body.temperature,
			voltage:     req.body.voltage
		}
	}])

	res.status(200).send()
})

app.listen(3000)
