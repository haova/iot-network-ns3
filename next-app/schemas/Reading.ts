import { z } from 'zod'

const Reading = z.object({
    sensor_name: z.string(),
    pdr: z.number(),
    rss: z.number(),
    updated_at: z.coerce.date(),
})

const ArrayOfReadings = z.array(Reading)

export { Reading, ArrayOfReadings }
