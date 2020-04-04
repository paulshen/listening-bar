type t = {
  id: string,
  accessToken: string,
  refreshToken: string,
  tokenExpireTime: float,
  anonymous: bool,
};

type session = {
  id: string,
  userId: string,
};