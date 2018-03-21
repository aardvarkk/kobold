var express = require('express')
var app = express()
var bodyParser = require('body-parser')
var passport = require('passport')
var LocalStrategy = require('passport-local').Strategy
var bcrypt = require('bcrypt')

const Sequelize = require('sequelize');
const sequelize = new Sequelize('kobold', 'aardvarkk', null, {
  dialect: 'postgres'
});

sequelize
  .authenticate()
  .then(() => {
    console.log('Connection has been established successfully.');
  })
  .catch(err => {
    console.error('Unable to connect to the database:', err);
  })

const User = sequelize.define('user', {
  email: { type: Sequelize.TEXT, allowNull: false, primaryKey: true },
  password: { type: Sequelize.TEXT, allowNull: false }
}, {
	timestamps: false
})

const Influx = require('influx')
const influx = new Influx.InfluxDB({
	host: 'localhost',
	database: 'kobold'
})

app.set('view engine', 'ejs')
app.use(bodyParser.json())
app.use(bodyParser.urlencoded())

passport.use(new LocalStrategy({ usernameField: 'email' },
  function(email, password, done) {
    User.findOne({ where: { email: email }}).then(user => {
	  	done(user.email)
      // if (err) { return done(err) }
      // if (!user) {
      //   return done(null, false, { message: 'Incorrect email.' })
      // }
      // if (!user.validPassword(password)) {
      //   return done(null, false, { message: 'Incorrect password.' })
      // }
      // return done(null, user)
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
