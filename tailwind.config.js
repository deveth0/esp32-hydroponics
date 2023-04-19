const config = {
  content: ["./dist/*.html", "./web/**/*.{html,js,ts}"],
  theme: {
    fontFamily: {
      sans: ["Roboto", "sans-serif"],
      serif: ["Roboto Slab", "serif"],
    },
    extend: {
      colors: {
        primary: "#31dbda",
        secondary: "#db31db",
      },
    },
  },
};

export default config;
