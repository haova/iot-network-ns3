const { createServer } = require('http')
const { parse } = require('url')
const next = require('next')
const db = require('./lib/db.js')
const app = next({ dev: true })
const { WebSocketServer } = require('ws')
const handle = app.getRequestHandler()

app.prepare().then(() => {
    const server = createServer((req, res) => handle(req, res, parse(req.url, true)))

    const wss = new WebSocketServer({ port: process.env.WEBSOCKET_PORT });

    wss.on('connection', function connection(ws) {
      db.allDocs({include_docs: true})
        .then((result) => {
          const warning = result.rows.some(row => row.doc.pdr > 3);
              ws.send(JSON.stringify({
                sensors: result.rows,
                isWarning: warning
              }))
        })
    
      db.changes({
        since: 'now',
        live: true,
      }).on("change", function() {
        db.allDocs({include_docs: true})
          .then((result) => {
              const warning = result.rows.some(row => row.doc.pdr > 3);
              ws.send(JSON.stringify({
                sensors: result.rows,
                isWarning: warning
              }))
          })
      })
    });


    server.listen(3000, () => {
        console.log(`> Ready on http://localhost:${3000} and ws://localhost:${process.env.WEBSOCKET_PORT}`)
    })
})


