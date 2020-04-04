const dotenv = require("dotenv");
const path = require("path");
const webpack = require("webpack");

dotenv.config();

const isProd = process.env.NODE_ENV === "production";
const outputDir = path.join(__dirname, "build");

module.exports = {
  entry: "./client/Index.bs.js",
  mode: isProd ? "production" : "development",
  output: {
    path: outputDir,
    filename: "index.js",
  },
  plugins: [
    new webpack.DefinePlugin({
      SPOTIFY_CLIENT_ID: JSON.stringify(process.env.SPOTIFY_CLIENT_ID),
    }),
  ],
  devServer: {
    compress: true,
    contentBase: outputDir,
    port: process.env.PORT || 8000,
    historyApiFallback: true,
  },
};
