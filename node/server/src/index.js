import Koa from 'koa'
import { koaBody } from 'koa-body'

const app = new Koa();

app.use(koaBody());
app.use(async ctx => {
  console.log(`[${new Date().toISOString()}] ${ctx.method} ${ctx.path}\n${JSON.stringify(ctx.request.body)}`)
  ctx.body = 'ok';
});

app.listen(8080, () => {
  console.log(`Server is running at port ${8080}`)
});