# trans2quik_wrapper

trans2quik.dll wrapper - includes a windows app (server) and linux dll (client). Used to control Quik running in Linux environment with help of Wine via Quik Transactions Import API. Server app launches in wine, wraps over trans2quik.dll and provides interface to invoke its functions via TCP socket. Both parts of wrapper are based on Qt 5.x framework. This is a beta version and not properly tested yet. Use at own risk.
