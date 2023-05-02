const config = {
  content: ["./dist/*.html", "./web/**/*.{html,js,ts}"],
  darkMode: "class",
  theme: {
    fontFamily: {
      sans: ["Roboto", "sans-serif"],
      serif: ["Roboto Slab", "serif"],
    },
    extend: {
      colors: {
        background: "#121212",
        primary: "#31dbda",
        secondary: "#db31db",
      },
    },
  },
};

export default config;
