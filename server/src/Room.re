type connection = {
  id: string,
  userId: string,
};

type t = {
  id: string,
  connections: array(connection),
};