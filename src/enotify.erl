-module(enotify).
-export([start/0, msg/0, msg/3]).


start() -> start("notify_drv").
start(Driver) ->
    case erl_ddll:load_driver(".", Driver) of
        ok -> ok;
        {error, already_loaded} -> ok;
        {error, Error} -> 
	    io:format("~p~n", [erl_ddll:format_error(Error)]),
	    exit({error, could_not_load_driver})
    end,
    spawn(fun() -> init() end).



init() ->
    register(?MODULE, self()),
    Port = open_port({spawn, "notify_drv"}, []),
    loop(Port).

loop(Port) ->
    receive
	{send, Tm, Label, Msg} -> 
	    send(Port, Tm, Label, Msg),
	    loop(Port)
    end.



msg() -> msg(200, "label", "message").
msg(Tm, Label, Msg) ->
    ?MODULE ! {send, Tm, Label, Msg},
    ok.


send(Port, Tm, Label, Msg) when is_list(Msg) ->
    Bll = length(Label),
    Blm = bytes(length(Msg)),
    Btm = bytes(Tm),
    Data =  Btm ++ [Bll] ++ Blm ++ Label ++ Msg,
    io:format("Data to be sent: ~p~n", [Data]),
    Port ! {self(), {command, Data}},
    receive
	Data -> io:format("port sent: ~p~n", [Data]),
	ok
    after 10 ->
	no_data
    end.
	

bytes(Value) ->
    B0 = Value band 255,
    B1 = (Value bsl 8) band 255,
    B2 = (Value bsl 16) band 255,
    B3 = (Value bsl 24) band 255,
    [B3, B2, B1, B0].
