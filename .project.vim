" Build troop-of-olde-cpp
let g:tmux_session = "dev"
let g:tmux_build_pane  = "0.0"
let g:tmux_server_pane = "0.1"
let g:tmux_client_pane = "0.2"
let g:build_command    = "cmake --build build"
set makeprg=cmake\ --build\ build

function! FindProjectRoot()
  let l:root = findfile('CMakeLists.txt', expand('%:p:h') . ';')
  if empty(l:root)
    echohl ErrorMsg | echo "CMakeLists.txt not found" | echohl None
    return ''
  endif
  return fnamemodify(l:root, ':h')
endfunction

function! Rebuild()
  let l:root = FindProjectRoot()
  let l:cwd = getcwd()
  if empty(l:root)
    return
  endif
  try
    " Run build from project root
    execute 'lcd ' . fnameescape(l:root)
    silent execute '!' . g:build_command
    let l:success = v:shell_error == 0

    if !l:success
      echohl ErrorMsg | echo "Build failed" | echohl None
    endif
    " Go back to original cwd
  finally
    execute 'lcd ' . fnameescape(l:cwd)
  endtry
  return l:success
endfunction

function! RebuildAndRestart()
  if !Rebuild()
      return
  endif

  " Restart server
  call system('tmux send-keys -t dev:0.1 C-c')
  sleep 200m
  call system('tmux send-keys -t dev:0.1 -l "./server.exe"')
  call system('tmux send-keys -t dev:0.1 C-m')

  call system('tmux send-keys -t dev:0.2 C-c')
  sleep 200m
  call system('tmux send-keys -t dev:0.2 -l "./client.exe"')
  call system('tmux send-keys -t dev:0.2 C-m')


  " Remove black text over terminal
  redraw!
endfunction

nnoremap <leader>b :call Rebuild()<CR>
nnoremap <leader>r :call RebuildAndRestart()<CR>
