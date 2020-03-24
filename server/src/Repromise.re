let let_ = Promise.flatMap;

module Wrap = {
  let let_ = Promise.map;
};

module Js = {
  let let_ = (bsPromise, cb) => {
    let promise = bsPromise->Promise.Js.fromBsPromise->Promise.Js.toResult;
    Promise.flatMap(promise, cb);
  };
};

module JsExn = {
  let let_ = (bsPromise, cb) => {
    let promise = bsPromise->Promise.Js.fromBsPromise->Promise.Js.toResult;
    Promise.flatMap(promise, result => cb(Belt.Result.getExn(result)));
  };
};