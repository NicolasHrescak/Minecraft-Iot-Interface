const u = {};

u.fs = require('fs');

u.assert = function(condition, message) {
    if(!condition)
        throw new TypeError(message);
}

module.exports = u;
