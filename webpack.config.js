const path = require("path");
const HtmlWebpackPlugin = require("html-webpack-plugin");
const MiniCssExtractPlugin = require("mini-css-extract-plugin");
const CompressionPlugin = require("compression-webpack-plugin");
const CssMinimizerPlugin = require("css-minimizer-webpack-plugin");
const TerserPlugin = require("terser-webpack-plugin");
const mockerAPI = require("mocker-api");

module.exports = function (env, argv) {
  return {
    context: __dirname,
    entry: "./web/src/main.ts",
    devtool: "inline-source-map",
    output: {
      path: path.join(__dirname, "dist"),
      publicPath: "/",
      filename: "app.js",
    },
    resolve: {
      extensions: [".ts", ".js"],
      alias: {
        Services: path.resolve(__dirname, "web/src/services"),
      },
    },
    module: {
      rules: [
        {
          test: /\.tsx?$/,
          use: "ts-loader",
          exclude: /node_modules/,
        },
        {
          test: /\.s[ac]ss$/i,
          use: [MiniCssExtractPlugin.loader, "css-loader", "sass-loader", "postcss-loader"],
        },
      ],
    },
    plugins: [
      new HtmlWebpackPlugin({
        template: "./web/pages/index.html",
      }),
      new HtmlWebpackPlugin({
        template: "./web/pages/welcome.html",
        filename: "welcome.html",
      }),
      new HtmlWebpackPlugin({
        template: "./web/pages/settings.html",
        filename: "settings.html",
      }),
      new HtmlWebpackPlugin({
        template: "./web/pages/settings/sensors.html",
        filename: "settings/sensors.html",
      }),
      new HtmlWebpackPlugin({
        template: "./web/pages/settings/wifi.html",
        filename: "settings/wifi.html",
      }),
      new HtmlWebpackPlugin({
        template: "./web/pages/settings/mqtt.html",
        filename: "settings/mqtt.html",
      }),
      new HtmlWebpackPlugin({
        template: "./web/pages/settings/backup.html",
        filename: "settings/backup.html",
      }),
      new MiniCssExtractPlugin(),
      argv.mode === "production" &&
        new CompressionPlugin({
          include: /\.(js|css)$/,
        }),
    ].filter(n => n),
    optimization: {
      minimize: true,
      minimizer: [new TerserPlugin(), new CssMinimizerPlugin(), "..."],
    },
    devServer: {
      setupMiddlewares(middlewares, devServer) {
        mockerAPI(devServer.app, path.resolve("./web/config/mocker-api/config.js"), {
          changeHost: true,
          header: {
            "Access-Control-Allow-Headers":
              "Content-Type, X-Requested-With, Authorization, metadata, Ocp-Apim-Subscription-Key",
          },
        });
        return middlewares;
      },
      headers: {
        "Access-Control-Allow-Origin": "*",
        "Content-Security-Policy":
          "default-src 'self' data:; style-src 'self' 'unsafe-inline' fonts.googleapis.com; font-src fonts.gstatic.com; script-src 'self' 'unsafe-inline'",
      },
    },
  };
};
