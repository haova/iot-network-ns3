import { z } from 'zod'

const Reading = z.object({
    name: z.string(),
    pdr: z.array(z.number()),
    rssi: z.array(z.number()),
})

const ReadingsFromAP = z.object({
    ap: z.string(),
    at: z.number(),
    sensors: z.array(Reading)
})

export { Reading, ReadingsFromAP }
