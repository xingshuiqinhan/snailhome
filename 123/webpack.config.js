
var webpack = require('webpack')
    ,HtmlwebpackPlugin = require('html-webpack-plugin')
    ,packageInfo = require('./package.json')
    ,path = require('path')
    ,ROOT_PATH = path.resolve(__dirname)
    ,APP_PATH = path.resolve(ROOT_PATH, 'src')
    ,BUILD_PATH = path.resolve(ROOT_PATH, 'build/'+packageInfo.version)
    ,serverConfig = require('./dev.server.config.js');

var OpenBrowserPlugin = require('open-browser-webpack-plugin');

  
module.exports = {
    entry:APP_PATH,
    output:{
        path:BUILD_PATH,
        filename:'assets/bundle.js'
    },
    devServer:serverConfig,
    module: {
        loaders: [
            {
                test: /\.css$/,                
                use: ['style-loader', 'css-loader']
            },            
            {
                test: /.(png|jpe?g|gif|svg)(\?\S*)?$/,
                loader: 'url-loader',
                query: {
                    limit: '400000',
                    name: 'assets/img/[name]_[hash:7].[ext]'
                }
            },
            {
                test: /\.js?$/,
                loader: 'babel-loader',
                include: APP_PATH,
                query: {
                    presets: ['es2015']
                }
            }
        ]
    },
    plugins: [
        new OpenBrowserPlugin({}),
        new webpack.HotModuleReplacementPlugin(),
        new HtmlwebpackPlugin({
            inject:true,
            hash:true,
            template:'src/index.html',
            minify:{
                removeComments:true,
                collapseWhitespace:false
            }
        })
    ]
}