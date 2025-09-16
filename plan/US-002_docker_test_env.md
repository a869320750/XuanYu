# US-002: Docker 测试环境搭建

优先级：高
工作量：1 人天

验收标准：
- Dockerfile 支持 Ubuntu/CentOS（或变体）
- 包含 GmSSL、GoogleTest、pkg-config 等依赖
- 支持源码挂载用于开发（docker-compose）
- 包含性能测试和安全扫描工具
- docker-compose支持多容器测试
