import { z } from 'zod'

const Reading = z.object({
    name: z.string(),
    pdr: z.array(z.number()),
    rss: z.array(z.number()),
    at: z.number(),
})

const ArrayOfReadings = z.array(Reading)

export { Reading, ArrayOfReadings }
