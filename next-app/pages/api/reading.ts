import { NextApiRequest, NextApiResponse } from 'next'
import db from '../../lib/db'
import { ArrayOfReadings } from '../../schemas/Reading'

export default async function handler(req: NextApiRequest, res: NextApiResponse) {
    const { method } = req

    switch (method) {
        case 'POST':
            try {
                const response = ArrayOfReadings.safeParse(req.body)

                if (!response.success) {
                    const { errors } = response.error

                    return res.status(400).json({
                        error: { message: 'Invalid request', errors },
                    })
                }

                db.bulkDocs(response.data.map((reading: any): any => ({ ...reading, _id: reading.sensor_name + reading.updated_at })))

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
