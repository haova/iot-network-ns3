import { NextApiRequest, NextApiResponse } from 'next'
import db from '../../lib/db'
import { ReadingsFromAP } from '../../schemas/Reading'

export default async function handler(req: NextApiRequest, res: NextApiResponse) {
    const { method } = req

    switch (method) {
        case 'POST':
            try {
                const response = ReadingsFromAP.safeParse(req.body)

                if (!response.success) {
                    const { errors } = response.error

                    return res.status(400).json({
                        error: { message: 'Invalid request', errors },
                    })
                }
                
                const readings = response.data.sensors.flatMap((sensor, sensor_index) => sensor.pdr.map((pdr, index) => ({
                    ap: response.data.ap,
                    name: response.data.sensors[sensor_index].name,
                    pdr: pdr,
                    rssi: response.data.sensors[sensor_index].rssi[index],
                    at: response.data.at+index,
                    _id: response.data.ap + response.data.sensors[sensor_index].name + (response.data.at+index)
                })))

                db.bulkDocs(readings)

                res.status(200).end()
            } catch (e) {
                console.error(e)
                res.status(500).end()
            }
            break
        default:
            res.setHeader('Allow', ['POST'])
            res.status(405).send(`Method ${method} is not allowed.`)
            break
    }
}
