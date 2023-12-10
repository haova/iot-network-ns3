const { createServer } = require('http')
const { parse } = require('url')
const next = require('next')
const db = require('./lib/db.js')
const axios = require('axios')
const app = next({ dev: true })
const { WebSocketServer } = require('ws')
const handle = app.getRequestHandler()

app.prepare().then(() => {
    const server = createServer((req, res) => handle(req, res, parse(req.url, true)))

    const wss = new WebSocketServer({ port: process.env.WEBSOCKET_PORT })

    wss.on('connection', function connection(ws) {
        db.allDocs({ include_docs: true }).then((result) => {
            const readingsToPredict = result.rows.filter((row) => row.doc.isWarning == null)
            if (readingsToPredict.length > 0) {
                axios.post(process.env.MLSERVER_URL, readingsToPredict).then((response) => {
                    db.bulkDocs(response.data?.map((row) => row.doc))
                }).catch((err) => {
                    console.log(err)
                })
            }
            
            ws.send(
                JSON.stringify({
                    readings: result.rows,
                })
            )
        })

        db.changes({
            since: 'now',
            live: true,
        }).on('change', function () {
            db.allDocs({ include_docs: true }).then((result) => {
                const readingsToPredict = result.rows.filter((row) => row.doc.isWarning == null)
                if (readingsToPredict.length > 0) {
                    axios.post(process.env.MLSERVER_URL, readingsToPredict).then((response) => {
                        db.bulkDocs(response.data?.map((row) => row.doc))
                    }).catch((err) => {
                        console.log(err)
                    })
                }
                
                ws.send(
                    JSON.stringify({
                        readings: result.rows,
                    })
                )
            })
        })
    })

    server.listen(3000, () => {
        console.log(`> Ready on http://localhost:${3000} and ws://localhost:${process.env.WEBSOCKET_PORT}`)
    })
})
