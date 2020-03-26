type user = {
  id: string,
  accessToken: string,
};

type state = {
  sessionId: option(string),
  user: option(user),
};
type action =
  | Login(string, string, string)
  | UpdateUser(user)
  | Logout;

let sessionId = Dom.Storage.(localStorage |> getItem("sessionId"));

let api =
  Restorative.createStore({sessionId, user: None}, (state, action) => {
    switch (action) {
    | Login(sessionId, accessToken, userId) => {
        sessionId: Some(sessionId),
        user: Some({id: userId, accessToken}),
      }
    | UpdateUser(user) => {...state, user: Some(user)}
    | Logout => {sessionId: None, user: None}
    }
  });

let getSessionId = () => api.getState().sessionId;
let useUser = () => api.useStoreWithSelector(state => state.user, ());
let getUser = () => api.getState().user;

let login = (sessionId, accessToken, userId) =>
  api.dispatch(Login(sessionId, accessToken, userId));
let logout = () => api.dispatch(Logout);
let updateUser = (user: user) => {
  api.dispatch(UpdateUser(user));
};