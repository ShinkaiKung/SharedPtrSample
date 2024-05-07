import userNapi from 'libuser.so'


export class User {
  private _userAddonInstance: any;

  constructor(user: userNapi.UserOptions) {
    this._userAddonInstance = userNapi.createUser(user.id);
  }

  getId(): string {
    return userNapi.getId(this._userAddonInstance);
  }
}