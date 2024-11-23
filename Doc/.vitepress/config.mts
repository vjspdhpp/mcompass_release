import { defineConfig } from 'vitepress'

// https://vitepress.dev/reference/site-config
export default defineConfig({
  title: "MCompass",
  description: "A Compass From Minecraft",
  themeConfig: {
    nav: [
      { text: 'Home', link: '/' },
    ],

    sidebar: [
      {
        text: '目录',
        items: [
          { text: 'Readme', link: '/readme' },
          { text: 'API', link: '/api' },
          { text: 'PCB', link: '/pcb' }
        ]
      }
    ],

    socialLinks: [
      { icon: 'github', link: 'https://github.com/chaosgoo/mcompass' }
    ]
  }
})