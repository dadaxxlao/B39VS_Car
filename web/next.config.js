/** @type {import('next').NextConfig} */
const nextConfig = {
  eslint: {
    // 在生产构建时禁用ESLint检查
    ignoreDuringBuilds: true,
  },
};

module.exports = nextConfig; 