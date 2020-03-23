include Promise;
let let_ = Promise.flatMap;

module Wrap = {
  let let_ = Promise.map;
};