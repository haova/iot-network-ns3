/** @type {import('next').NextConfig} */
const { API_BASEURL, WEBSOCKET_URL } = process.env;
const nextConfig = {
  reactStrictMode: true,
  swcMinify: true,
  env: {
    API_BASEURL,
    WEBSOCKET_URL
  }
}

module.exports = nextConfig
