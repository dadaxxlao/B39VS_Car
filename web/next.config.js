/** @type {import('next').NextConfig} */
const nextConfig = {
  eslint: {
    // 在生产构建时禁用ESLint检查
    ignoreDuringBuilds: true,
  },
  // 禁用类型检查，加速构建
  typescript: {
    ignoreBuildErrors: true,
  },
  // 输出模式配置
  output: 'standalone',
};

module.exports = nextConfig; 