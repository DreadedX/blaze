import express from 'express';
import logger from 'morgan';

const app = express();

app.use(logger('short'));

express.static.mime.define({'application/wasm': ['wasm']});
express.static.mime.define({'application/flame': ['flm']});

// @todo These are just for testing right now, this needs to point to a better location
// @todo Easy toggle between release and debug
app.use('/static', express.static('../.flint/build/web/debug/bin'));
app.use('/static', express.static('../.flint/build/web/debug/archives'));
app.use('/', express.static('public'));

app.listen(3000, () => {
	console.log('Listening on port 3000')
});
