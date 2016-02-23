# This file must be used with "source bin/activate.csh" *from csh*.
# You cannot run it directly.
# Created by Davide Di Blasi <davidedb@gmail.com>.

alias deactivate 'test $?_OLD_VIRTUAL_PATH != 0 && setenv PATH "$_OLD_VIRTUAL_PATH" && unset _OLD_VIRTUAL_PATH; rehash; test $?_OLD_VIRTUAL_PROMPT != 0 && set prompt="$_OLD_VIRTUAL_PROMPT" && unset _OLD_VIRTUAL_PROMPT; unsetenv VIRTUAL_ENV; test "\!:*" != "nondestructive" && unalias deactivate && unalias pydoc'

# Unset irrelevant variables.
deactivate nondestructive

setenv VIRTUAL_ENV "/Users/olive/Documents/SourceCode/aria2/build-release"

set _OLD_VIRTUAL_PATH="$PATH"
setenv PATH "$VIRTUAL_ENV/bin:$PATH"



if ("" != "") then
    set env_name = ""
else
    if (`basename "$VIRTUAL_ENV"` == "__") then
        # special case for Aspen magic directories
        # see http://www.zetadev.com/software/aspen/
        set env_name = `basename \`dirname "$VIRTUAL_ENV"\``
    else
        set env_name = `basename "$VIRTUAL_ENV"`
    endif
endif

# Could be in a non-interactive environment,
# in which case, $prompt is undefined and we wouldn't
# care about the prompt anyway.
if ( $?prompt ) then
    set _OLD_VIRTUAL_PROMPT="$prompt"
    set prompt = "[$env_name] $prompt"
endif

unset env_name

alias pydoc python -m pydoc

rehash

