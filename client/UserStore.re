type user = {id: string};

type state = {
  sessionId: option(string),
  accessToken: option(string),
  user: option(user),
};
type action =
  | Login(string, string)
  | FetchUser(user);

let sessionId = Dom.Storage.(localStorage |> getItem("sessionId"));
let accessToken = Dom.Storage.(localStorage |> getItem("accessToken"));

let api =
  Restorative.createStore(
    {sessionId, accessToken, user: None}, (state, action) => {
    switch (action) {
    | Login(sessionId, accessToken) => {
        sessionId: Some(sessionId),
        accessToken: Some(accessToken),
        user: None,
      }
    | FetchUser(user) => {...state, user: Some(user)}
    }
  });

let getSessionId = () => api.getState().sessionId;
let useUser = () => api.useStoreWithSelector(state => state.user, ());

let setUser = (user: user) => api.dispatch(FetchUser(user));