import { NextApiRequest, NextApiResponse } from 'next'
import db from '../../lib/db'
import { Reading } from '../../schemas/Reading'

export default async function handler(req: NextApiRequest, res: NextApiResponse) {
    const { method } = req

    switch (method) {
        case 'POST':
            try {
                const response = Reading.safeParse(req.body)

                if (!response.success) {
                    const { errors } = response.error

                    return res.status(400).json({
                        error: { message: 'Invalid request', errors },
                    })
                }
                
                const readings = response.data.pdr.map((pdr, index) => ({
                    name: response.data.name,
                    pdr: pdr,
                    rss: response.data.rss[index],
                    at: response.data.at+index,
                    _id: response.data.name + (response.data.at+index)
                }))

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
