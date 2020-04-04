type user = {
  id: string,
  accessToken: string,
  anonymous: bool,
};

type state = {
  sessionId: option(string),
  user: option(user),
  hasFetched: bool,
};
type action =
  | Login(string, string, string, bool)
  | UpdateUser(user)
  | SetAnonymous(bool)
  | Logout
  | FetchFail;

let sessionId = Dom.Storage.(localStorage |> getItem("sessionId"));

let api =
  Restorative.createStore(
    {sessionId, user: None, hasFetched: false}, (state, action) => {
    switch (action) {
    | Login(sessionId, accessToken, userId, anonymous) => {
        sessionId: Some(sessionId),
        user: Some({id: userId, accessToken, anonymous}),
        hasFetched: true,
      }
    | UpdateUser(user) => {...state, user: Some(user), hasFetched: true}
    | SetAnonymous(anonymous) => {
        ...state,
        user: Belt.Option.map(state.user, user => {...user, anonymous}),
      }
    | Logout => {...state, sessionId: None, user: None}
    | FetchFail => {...state, user: None, hasFetched: true}
    }
  });

let getSessionId = () => api.getState().sessionId;
let useHasFetched = () =>
  api.useStoreWithSelector(state => state.hasFetched, ());
let useUser = () => api.useStoreWithSelector(state => state.user, ());
let getUser = () => api.getState().user;

let login = (sessionId, accessToken, userId, anonymous) =>
  api.dispatch(Login(sessionId, accessToken, userId, anonymous));
let logout = () => api.dispatch(Logout);
let updateUser = (user: user) => {
  api.dispatch(UpdateUser(user));
};
let setAnonymous = (anonymous: bool) => {
  api.dispatch(SetAnonymous(anonymous));
};
let fetchFail = () => api.dispatch(FetchFail);