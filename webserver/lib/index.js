import express from 'express';
import logger from 'morgan';

const app = express();

app.use(logger('short'));

express.static.mime.define({'application/wasm': ['wasm']});

// @todo These are just for testing right now, this needs to point to a better location
app.use('/static', express.static('../.flint/build/web/bin'));
app.use('/static', express.static('../build/archives'));
app.use('/', express.static('public'));

app.listen(3000, () => {
	console.log('Listening on port 3000')
});
