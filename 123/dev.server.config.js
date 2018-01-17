function Config(){
    
    var files = {
        '/users/*':{},
        '/file/*':{},
        '/order/*':{},
        '/product/*':{},
        '/operations/*':{},
        '/api/*':{},
    }

    for(var i in files){

        //files[i].target ='http://192.168.60.115/';
        //files[i].target ='http://192.168.60.116/';

        //files[i].target ="http://www.shoelives.com/"
        files[i].target ="http://icy.iidingyun.com/"
        files[i].changeOrigin = true;
    }
    return {
        port: 8080,
        disableHostCheck:true,
        historyApiFallback: true,
        hot: true,
        inline: true,
        stats: { colors: true },
        proxy: files
    }
}

module.exports =  Config();