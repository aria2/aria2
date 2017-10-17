.. _aria2c:

aria2c(1)
=========

SINOPSE
-------

**aria2c** [<OPÇÕES>] [<URI>|<MAGNET>|<ARQUIVO_TORRENT>|<ARQUIVO_METALINK>] ...

.. index::   double:  Descrição; Sumário
             double: Resumo; Executivo
             triple: O que; Por que; Porque
    	        

DESCRIÇÃO
---------

.. warning::

   This translation has been outdated quite sometime now, and lacks
   many recent changes.  Please consult English version manual for
   updated information.

Observação: Para executar o aria2 em um terminal ou no prompt da
linha de comando do windows, utilize o comando aria2c.

aria2 é um utilitário para download de arquivos. Os protocolos suportados são
HTTP, HTTPS, FTP, BitTorrent e Metalink. aria2 pode efetuar download de arquivos
a partir de múltiplas fontes e protocolos e tenta utilizar a máxima capacidade
de banda. Há suporte para download de arquivos que tem origem em HTTP, HTTPS,
FTP, BitTorrent e Metalink ao mesmo tempo, enquanto os dados baixados podem ser
(uploaded) e compartilhados pelo BitTorrent. Usando conferência / aferição (checksum) nos
Metalinks, aria2 automaticamente valida o conteúdo dos dados enquanto faz
o download do arquivo como BitTorrent.

.. index::   double: Opções; Básicas;

Opções Básicas do aria2
-----------------------

Opções Comuns
~~~~~~~~~~~~~

.. option:: -d, --dir=<DIR>

  O diretório onde será armazenado o arquivo baixado.

.. option:: -i, --input-file=<ARQUIVO>

  Executa download da(s) URI encontradas no ARQUIVO. Podem ser especificados
  múltiplos URI para uma simples entidade: separe URI na mesma linha com
  um caracter TAB (tabulação).
  Quando desejar ler a entrada padrão (stdin) especificar ``-`` (hífen).
  Adicionalmente, diversas opções podem ser especificadas após cada linha de URI.
  Esta(s) linha(s) opcional(is) deve(m) começar(em) com um ou mais espaços em
  branco e possuir(em) apenas uma opção por linha.
  Ver subseção `Arquivo de Entrada`_ para mais detalhes.
  Ver também opção :option:`--deferred-input`.

.. option:: -l, --log=<LOG>

  O nome do arquivo de log. Se ``-`` for especificado (entrada padrão), log é
  gravado em stdout (saída padrão). Se uma string vazia("") é especificada, log
  não será gravado em arquivo.

.. option:: -j, --max-concurrent-downloads=<N>

  Configura o número máximo de downloads paralelos para cada URI (HTTP,
  HTTPS, FTP), Torrent e Metalink. Ver também opção :option:`--split <-s>`.
  Padrão: ``5``

.. option:: -V, --check-integrity [true|false]

  Verifica a integridade do arquivo validando pedaços hashes ou um hash do
  arquivo inteiro.  Essa opção tem efeito só em downloads BitTorrent, Metalink
  com checksums ou HTTP, HTTPS e FTP com a opção :option:`--checksum`. Se pedaços
  de hashes são providos, essa opção pode detectar porções danificadas de um
  arquivo e efetuar novamente o download desses pedaços. Se especificar hash
  do arquivo inteiro, a verificação do hash ocorrerá só ao final do download,
  validação que leva em conta o tamanho do arquivo, e o download reinicirá a
  partir do início.  Se houver especificação de ambos métodos de hash será
  utilizado o hash de pedações.
  Padrão:
  ``false``

.. option:: -c, --continue [true|false]

   Continua o download a partir de um download parcial, anteriormente
   interrompido.  Use esta opção para retormar um download iniciado a partir
   de um browser (navegador) ou outro programa que faz baixa de arquivos 
   sequencialmente desde o início.
   Atualmente esta opção só é aplicavel a download de HTTP, HTTPS e FTP.

.. option:: -h, --help[=<TÓPICO>|<PALAVRA-CHAVE>]

   As mensagens de Ajuda são classificadas em temas. Um tema se inicia com
   ``#``. Por exemplo, digitar ``--help=#http`` para obter a explicação do uso das
   opções do tema ``#http``. Se digitar um termo que não é tema, haverá exibição
   das opções que incluem o termo informado.
   Valores disponíveis para temas podem ser: ``#basic``, ``#advanced``, 
   ``#http``, ``#https``, ``#ftp``, 
   ``#metalink``, ``#bittorrent``, ``#cookie``, ``#hook``, ``#file``, ``#rpc``,
   ``#checksum``, ``#experimental``, ``#deprecated``, ``#help``, ``#all``
   Padrão: ``#basic``

Opções HTTP / FTP
~~~~~~~~~~~~~~~~~

.. index::	double: proxy; servidor;
		triple: usuário; senha; proxy;

.. option:: --all-proxy=<PROXY>

  Usar este servidor proxy para todos protocolos.  Para limpar proxy
  previamente definido, use "".  Esta configuração pode ser sobreposta através
  da especificação de um servidor proxy para um determinado protocolo usando
  opções :option:`--http-proxy`, :option:`--https-proxy` e :option:`--ftp-proxy`.
  Isto afeta todas as URIs.
  O formato da opção PROXY é ``[http://][USUÁRIO:SENHA@]SERVIDOR[:PORTA]``.
  Ver também seção `VARIÁVEIS DE AMBIENTE`_  section.

  .. note::
    
    Se usuário e senha são embutidos na URI do proxy eles também podem ser
    especificados através das opções
    *--{http,https,ftp,all}-proxy-{usuário,senha}*, 
    aqueles que aparecerem por último assumem a precedência.
    Por exemplo, quando temos: 
    ``http-proxy-user=meunome``, ``http-proxy-passwd=minhasenha`` 
    em aria2.conf e especificamos ``--http-proxy="http://svrproxy"`` na
    linha de comando, então obtemos como proxy HTTP ``http://svrproxy`` 
    com o usuário ``meunome`` e senha ``minhasenha``.

    Outro exemplo: quando especificamos na linha de comando:
    ``--http-proxy="http://usuário:senha@svrproxy" --http-proxy-user="meunome" 
    --http-proxy-passwd="minhasenha"``, então obtemos proxy HTTP 
    ``http://svrproxy`` com usuário ``meunome`` e senha ``minhasenha``.

    Mais um exemplo: se especificamos na linha de comando:
    ``--http-proxy-user="meunome" --http-proxy-passwd="minhasenha" 
    --http-proxy="http://utilizador:acesso@svrproxy"``, então obtemos 
    o proxy HTTP ``http://svrproxy`` com o usuário ``utilizador``
    e a senha ``acesso``.

.. option:: --all-proxy-passwd=<SENHA>

  Define senha para a opção :option:`--all-proxy`.

.. option:: --all-proxy-user=<USUÁRIO>

  Define usuário para opção :option:`--all-proxy`.

.. option:: --checksum=<TIPO>=<ALGORITMO>

  Define verificação (checksum). TIPO é o tipo de algoritmo(hash). Os tipos de
  algoritmos estão listados em ``Algoritmos de Hash`` e podem ser obtidos
  através do do comando ``aria2c -v``. DIGEST é o código hexadecimal.  Por
  examplo, definindo sha-1 o resultado parece com:
  ``sha-1=0192ba11326fe2298c8cb4de616f4d4140213838`` Essa opção aplica-se
  apenas para downloads HTTP, HTTPS e FTP.

.. option:: --connect-timeout=<SEGUNDOS>

  Define o tempo de espera em segundos para estabelecer a conexão com o servidor
  proxy. Após o estabelecimento da conexão, esta opção não tem mais efeito, mas
  a opção :option:`--timeout <-t>` será utilizada.
  Padrão: ``60``

.. option:: --dry-run [true|false]

  Se ``true`` é informado, aria2 apenas verifica se o arquivo remoto está 
  disponível para download dos dados. Esta opção tem efeito em downloads de
  servidores HTTP, HTTPS e FTP.  Downloads de BitTorrent serão cancelados se for 
  especificado ``true``.
  Padrão: ``false``

.. option:: --lowest-speed-limit=<VELOCIDADE>

  Fecha a conexão se a velocidade de download é menor ou igual ao valor
  especificado, bytes por segundo.
  ``0`` significa que aria2 não levará em conta limite de velocidade mínima.
  Pode ser anexado ``K`` ou ``M`` (1K = 1024, 1M = 1024K).
  Esta opção não abrange downloads do tipo BitTorrent.
  Padrão: ``0``

.. option:: -x, --max-connection-per-server=<NÚMERO>

  O número máximo de conexões para um servidor em cada download.
  Padrão: ``1``

.. option:: --max-file-not-found=<NÚMERO>

  Se aria2 recebe çódigo de retorno "arquivo não encontrado" de um servidor
  remoto de HTTP / FTP um NÚMERO de vezes sem obter nenhum byte, então o
  download é forçado a falhar.
  Especificar ``0`` para desabilitar esta opção. Esta opção só é válida
  para servidores HTTP / FTP.
  Padrão: ``0``

.. option:: -m, --max-tries=<NÚMERO>

  Define o número de tentativas. ``0`` significa ilimitadas.
  See also :option:`--retry-wait`.
  Padrão: ``5``

.. option:: -k, --min-split-size=<TAMANHO>

  aria2 não divide menos que 2 * TAMANHO o intervalo de bytes.  Por exemplo,
  considere download de um arquivo de 20MiB. Se o TAMANHO é 10M, aria2 pode
  dividir o arquivo em 2 intervalos de [0-10MiB) e [10MiB-20MiB) e executar o 
  download usando 2 fontes (logicamente se a opção :option:`--split <-s>` >= 2).
  Se o TAMANHO é 15M, desde que 2 * 15M > 20Mib, aria2 não dividirá o arquivo e
  fará o download de 1 fonte.  Pde ser anexado ``K`` ou ``M`` 
  (1K = 1024, 1M = 1024K).
  Valores Possíveis: ``1M`` -``1024M`` 
  Padrão: ``20M``

.. option:: -n, --no-netrc [true|false]

  Desabilita suporte netrc.
  Padrão: Suporte a netrc é habilitado por padrão.

  .. note::
    
    arquivo netrc é lido somente no início se a opção :option:`--no-netrc <-n>` é
    ``false``.
    Portanto se a opção :option:`--no-netrc <-n>` é ``true`` no início, não haverá
    netrc disponível durante toda a sessão, mesmo que seja utilizada a opção
    :func:`aria2.changeGlobalOption` para executar a opção :option:
    `--no-netrc=false <-n>`.
    .

.. option:: --no-proxy=<DOMÍNIOS>

  Especifica nomes de servidores, domínios e endereços de redes com ou sem blocos
  CIDR para os quais não serão utilizados proxy.

  .. note::

    Para endereço de rede com blocos CIDR, ambos endereços IPv4 ou IPv6 funcionam. 
    Implementação atual, não resolve nome host em URI para comparar com endereço 
    especificado na opção :option:`--no-proxy`. Portanto só será efetiva se a URI 
    possuir números de endereço IP.

.. option:: -o, --out=<ARQUIVO>

  O nome do arquivo baixado. Quando a opção :option:`--force-sequential <-Z>` é 
  utilizada esta opção será ignorada.

  .. note::

    Em um download Metalink ou BitTorrent não poderá ser especificado o nome
    do arquivo. O nome do arquivo especificado aqui é usado quando através
    da linha de comando é informada para o aria2 sem a utilização da opção
    :option:`--input-file <-i>`, :option:`--force-sequential <-Z>`.
    Por exemplo:

    .. code-block:: console

      $ aria2c -o meuarquivo.zip "http://server1/arquivo.zip" "http://server2/arquivo.zip"

.. option:: --proxy-method=<MÉTODO>

  Define o método utilizado para requisições de proxy.  MÉTODO é ``get`` ou
  ``tunnel``. Downloads HTTPS sempre utiliza ``tunnel``, independentemente
  desta opção.
  Padrão: ``get``

.. option:: -R, --remote-time [true|false]

  Recuperar timestamp do arquivo remoto a partir do servidor remoto HTTP / FTP
  e se disponível, aplicá-lo ao arquivo local.
  Padrão: ``false``

.. option:: --reuse-uri [true|false]

  Reutilizar uma URI já utilizada. Se não habilitada as URIs já utilizadas serão 
  abandonadas.
  Padrão: ``true``

.. option:: --retry-wait=<SEGUNDOS>

  Define quantos segundos haverá entre as tentativas. Com SEGUNDOS > 0, aria2 irá
  tentará fazer o download quando o servidor HTTP retornar código resposta 503.
  Padrão:  ``0``

.. option:: --server-stat-of=<ARQUIVO>

  Define o nome do arquivo no qual será salvo o perfil de performance de um
  ou mais servidores acessados.
  Para carregar dados já salvos utilizar opção :option:`--server-stat-if`.
  Ver subseção `Perfil Performance Servidor`_ abaixo,
  para o formato do arquivo.
    

.. option:: --server-stat-if=<ARQUIVO>

  Specify the filename to load performance profile of the servers. The
  loaded data will be used in some URI selector such as ``feedback``.
  See also :option:`--uri-selector` option. See
  `Perfil Performance Servidor`_
  subsection below for file format.

.. option:: --server-stat-timeout=<SEGUNDOS>

  Specifies timeout in seconds to invalidate performance profile of
  the servers since the last contact to them.
  Padrão: ``86400`` (24hours)

.. option:: -s, --split=<N>

  Download a file using N connections.  If more than N URIs are given,
  first N URIs are used and remaining URIs are used for backup.  If
  less than N URIs are given, those URIs are used more than once so
  that N connections total are made simultaneously.  The number of
  connections to the same host is restricted by
  :option:`--max-connection-per-server <-x>` option.
  See also :option:`--min-split-size <-k>` option.
  Padrão: ``5``

  .. note::
    
    Some Metalinks regulate the number of servers to connect.  aria2
    strictly respects them.  This means that if Metalink defines the
    maxconnections attribute lower than N, then aria2 uses the
    value of maxconnections attribute instead of N.

.. option:: --stream-piece-selector=<SELECTOR>

  Specify piece selection algorithm used in HTTP e FTP download. Piece
  means fixed length segment which is downloaded in parallel in
  segmented download. If ``default`` is given, aria2 selects piece so
  that it reduces the number of establishing connection. This is
  reasonable default behaviour because establishing connection is an
  expensive operation.  If ``inorder`` is given, aria2 selects piece
  which has minimum index. Index=0 means first of the file. This will
  be useful to view movie while downloading it.
  :option:`--enable-http-pipelining` option may
  be useful to reduce reconnection overhead.  Please note that aria2
  honors
  :option:`--min-split-size <-k>` option,
  so it will be necessary to specify a reasonable value to
  :option:`--min-split-size <-k>` option.
  If ``geom`` is given, at the beginning aria2 selects piece which has
  minimum index like ``inorder``, but it exponentially increasingly
  keeps space from previously selected piece. This will reduce the
  number of establishing connection and at the same time it will
  download the beginning part of the file first. This will be useful
  to view movie while downloading it.
  Padrão: ``default``

.. option:: -t, --timeout=<SEGUNDOS>

  Set timeout in seconds.
  Padrão: ``60``

.. option:: --uri-selector=<SELECTOR>

  Specify URI selection algorithm. The possible values are ``inorder``,
  ``feedback`` and ``adaptive``.  If ``inorder`` is given, URI is tried in
  the order appeared in the URI list.  If ``feedback`` is given, aria2
  uses download speed observed in the previous downloads and choose
  fastest server in the URI list. This also effectively skips dead
  mirrors. The observed download speed is a part of performance
  profile of servers mentioned in :option:`--server-stat-of` and
  :option:`--server-stat-if` options.  If ``adaptive`` is given, selects one of
  the best mirrors for the first and reserved connections.  For
  supplementary ones, it returns mirrors which has not been tested
  yet, and if each of them has already been tested, returns mirrors
  which has to be tested again. Otherwise, it doesn't select anymore
  mirrors. Like ``feedback``, it uses a performance profile of servers.
  Padrão: ``feedback``

Opções Específicas de HTTP e HTTPS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. index:	triple: hhtp; http; opções

.. option:: --ca-certificate=<ARQUIVO>

  Utilizar o certificado do ARQUIVO FILE para verificar os Servidores.
  O certificado precisa estar no formato PEM e pode conter múltiplos
  certificados CA.
  
  Utilizar a opção :option:`--check-certificate` para habilitar a verificação.

  .. note::

    Se aria2 foi compilado com OpenSSL ou versão mais recente de GnuTLS a qual
    tem a função ``gnutls_certificate_set_x509_system_trust()`` e a biblioteca
    foi adequadamente configurada para localizar o certificado CA existente,
    aria2 irá carregar automaticamente estes certificados no início.

.. option:: --certificate=<ARQUIVO>

  Usar arquivo com certificado cliente.
  O certificado deve estar no formato PEM.
  Pode ser usada a opção :option:`--private-key` para especificar uma chave
  particular.

.. option:: --check-certificate [true|false]

  Verifica se o peer esta usando o certificado especificado na opção
  :option:`--ca-certificate`.
  Padrão: ``true``

.. option:: --http-accept-gzip [true|false]

  Envia cabeçalho requisição ``Accept: deflate, gzip`` e faz (inflate) se
  o servidor remoto responder com  ``Content-Encoding: gzip`` ou 
  ``Content-Encoding: deflate``.  Padrão: ``false``

  .. note::
    
    Alguns servidores respondem com ``Content-Encoding: gzip`` para arquivos
    que são gzip. aria2 faz inflate destes por causa do cabeçalho de resposta.

.. option:: --http-auth-challenge [true|false]

  Envia cabeçalho de autorização HTTP só quando requisitado pelo servidor.
  Se ``false`` é habilitado, então o cabeçalho de autorização sempre será
  enviado ao servidor.  Há uma exceção: se o nome do usuário de senha são
  embutidas na URI, o cabeçalho de autorização sempre será enviado ao servidor
  independente desta opção.  Padrão: ``false``

.. option:: --http-no-cache [true|false]

   Envia Cache-Control: no-cache e Pragma: cabeçalho no-cache para evitar
   conteúdo do cache.  Se ``false`` é fornecido, esses cabeçalhos não serão
   enviados e poderá ser adicionado o cabeçalho de Cache-Control com a diretiva
   desejada usando a opção :option:`--header`. Padrão: ``true``

.. option:: --http-user=<USUÁRIO>

  Define usuário HTTP. Isto afeta todas as URIs.

.. option:: --http-passwd=<PASSWD>

  Define senha HTTP. Isto afeta todas as URIs.

.. option:: --http-proxy=<PROXY>

  Usar este servidor proxy para HTTP.  Para limpar o proxy anteriormente
  definido use "".  Ver também opção :option:`--all-proxy`.  Isto afeta todas
  URIs.  O formato de PROXY é ``[http://][USUÁRIO:SENHA@]SERVIDOR[:PORTA]``

.. option:: --http-proxy-passwd=<SENHA>

  Define a senha para opção :option:`--http-proxy`.

.. option:: --http-proxy-user=<USUÁRIO>

  Define o usuário para a opção :option:`--http-proxy`.

.. option:: --https-proxy=<PROXY>

  Usar este servidor proxy para HTTPS. Para limpar o proxy anteriormente,
  use "".  Ver também opção :option:`--all-proxy`.  Isto afeta todas URIs.  O
  formato de PROXY é ``[https://][USUÁRIO:SENHA@]SERVIDOR[:PORTA]``

.. option:: --https-proxy-passwd=<SENHA>

  Define senha para a opção :option:`--https-proxy`.

.. option:: --https-proxy-user=<USUÁRIO>

  Define usuário para a opção :option:`--https-proxy`.

.. option:: --private-key=<ARQUIVO>

  Define o arquivo de chave particular que será usado.
  A chave particular deve estar no formato PEM e não pode estar criptografada.
  O comportamento quando estiver criptografada é indefinido.
  Ver também a opção :option:`--certificate`.

.. option:: --referer=<REFERER>

  Define a referência. Afeta todas URIs. Se ``*`` é usado, cada URI requisitada é usada
  como referência (referer). Pode ser útil quando usado em conjunto com a opção
  :option:`--parameterized-uri`. 

.. option:: --enable-http-keep-alive [true|false]

  Enable HTTP/1.1 persistent connection.
  Padrão: ``true``

.. option:: --enable-http-pipelining [true|false]

  Habilita pipelining para HTTP/1.1.
  Padrão: ``false``

  .. note::
    
    Da perspectiva de performance, não há vantagem em habilitar esta opção.

.. option:: --header=<HEADER>

  Anexa CABEÇALHOao ao cabeçalho HTTP requisitado.
  Pode usar esta opção várias vezes para especificar múltiplos cabeçalhos:

  .. code-block:: console

        $ aria2c --header="X-A: b78" --header="X-B: 9J1" "http://servidor/arquivo"

.. index::	triple: cookies; load; save;

.. option:: --load-cookies=<ARQUIVO>

  Carregar Cookies do ARQUIVO usando formato Firefox3 format (SQLite3),
  Chromium / Google Chrome (SQLite3) e formato
  Mozilla / Firefox(1.x/2.x) / Netscape.

  .. note::

    Se aria2 é compilado sem libsqlite3, então não havera suporte aos formatos 
    de cookie Firefox3 e Chromium / Google Chrome.

.. option:: --save-cookies=<ARQUIVO>

  Salva Cookies para o ARQUIVO no formato Mozilla / Firefox(1.x/2.x) / 
  Netscape.  Se ARQUIVO já existe, será sobreposto.  Cookies da Sessão também
  serão salvos e seus valores de expiração serão tratados como 0.  
  Valores Possíveis: ``/caminho/do/arquivo``

.. option:: --use-head [true|false]

  Usar método HEAD para a primeira requisição ao servidor HTTP.
  Padrão: ``false``


.. option:: -U, --user-agent=<AGENTE_USUÁRIO>

  Define usuário agente para download HTTP, HTTPS.
  Padrão: ``aria2/$VERSION``, $VERSION é substituída pela versão do aria2.

Opções Específicas de FTP
~~~~~~~~~~~~~~~~~~~~~~~~~

.. option:: --ftp-user=<USUÁRIO>

  Definir o usuário FTP. Isto afeta todas as URIs.
  Padrão: ``anonymous``

.. option:: --ftp-passwd=<SENHA_FTP>

  Definir senha FTP. Isto afeta todas as URIs.
  Se o nome existe, mas a senha esta ausente, para login em uma URI, aria2
  tenta obter a senha usando o arquivo .netrc, caso exista senha declarada
  no .netrc. Se não existir será utilizada a senha declarada nesta opção.
  Padrão: ``ARIA2USER@``

.. option:: -p, --ftp-pasv [true|false]

  Usar modo passivo no FTP.
  Se ``false`` é informado, o modo ativo será usado.
  Padrão: ``true``

.. option:: --ftp-proxy=<PROXY>

  Usar este servidor proxy para FTP.  Para limpar definição proxy previamente
  definido, use "".  Ver também opção :option:`--all-proxy`.
  Isto afeta todas URIs.  O formato do PROXY é
  ``[http://][USUÁRIO:SENHA@]SERVIDOR[:PORTA]``

.. option:: --ftp-proxy-passwd=<PASSWD>

  Define senha para a opção :option:`--ftp-proxy`.

.. option:: --ftp-proxy-user=<USUÁRIO>

  Define senha para opção :option:`--ftp-proxy`.

.. option:: --ftp-type=<TYPE>

  Define tipo de transferência FTP. Que pode ser: ``binary`` ou ``ascii``.
  Padrão: ``binary``

.. option:: --ftp-reuse-connection [true|false]

  Reutilizar conexão FTP.
  Padrão: ``true``

Opções Comuns de BitTorrent / Metalink
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. option:: --select-file=<INDEX>...

  Define arquivo para download através da especificação de seu index.
  Para achar o index do arquivo use a opção :option:`--show-files <-S>`.
  Múltiplos indíces podem ser especificados usando-se ``,``, por exemplo:
  ``3,6``.  Também pode ser usado ``-`` para especificar intervalos:
  ``1-5``.  Ambos podem ser usados juntos ``,`` e ``-`` exemplo:
  ``1-5,8,9``.  Quando usados com a opção -M, o índice pode variar dependendo
  das opções da query. Ver opções (*--metalink-\ **).

  .. note::

    Em torrent de múltiplos arquivos, os arquivos adjacentes especificados
    por essa opção também podem ser baixados. Esse é o comportamento esperado
    não é um bug/erro.  Um simples pedaço pode incluir diversos arquivos ou
    partes de arquivos, e aria2 grava o pedaço(s) no(s) arquivo(s)
    apropriado(s).

.. option:: -S, --show-files [true|false]

  Imprimir a lista de arquivos do ".torrent", ".meta4" e ".metalink" e termina.
  No caso de arquivo ".torrent", informações adicionais são impressas.
  (infohash, tamanho pedaço, etc).
  
Opções Específicas de BitTorrent
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. option:: --bt-enable-lpd [true|false]

  Habilita Descobrir Peer Local.  Se indicador particular é configurado 
  no torrent, aria2 não usa esta funcionalidade mesmo que ``true`` foi
  informato.
  Padrão: ``false``

.. option:: --bt-exclude-tracker=<URI>[,...]

  Lista separada por vírgulas, de trackers de URI BitTorrent que devem ser
  removidas.  Pode ser usado o valor especial ``*`` para especificar todas
  URIs; isso irá excluir todas URIs de "announce".  Quando especificar ``*`` 
  em uma linha de comando do shell, lembre-se de forçar o escape or delimite
  com aspas, apóstrofo ou em linux com crase caracter `.
  Ver também opção :option: `--bt-tracker`.

.. option:: --bt-external-ip=<ENDEREÇO-IP>

  Especificar o IP externo para reportar um track BitTorrent.  Mesmo que esta
  função tenha o nome ``external``, ela pode aceitar qualquer tipo de endereço
  IP.  ENDEREÇO-IP deve ser um endereço IP numérico.

.. option:: --bt-hash-check-seed [true|false]

 Se ``true`` é informado, após o check do hash usando a opção :option: 
 `--check-integrity <-V>` e o arquivo esta completo, continue o arquivo seed.
 Se desejar verificar o arquivo e efetuar o download somente quando ele estiver
 imcompleto ou danificado, defina esta opção para ``false``.  Esta opção
 só tem efeito para download de BitTorrent.
 Padrão: ``true``

.. option:: --bt-lpd-interface=<INTERFACE>

  Use o interface de rede informado para Descobrir o Peer Local. Se esta opção
  não é especificada, o interface padrão é usado. Pode ser especificado o nome
  do interface e o endereço IP.
  Valores possíveis: interface, endereço IP

.. option:: --bt-max-open-files=<NÚMERO>

  Especificar o número máximo de arquivos para abrir para cada download
  BitTorrent.
  Padrão: ``100``

.. option:: --bt-max-peers=<NÚMERO>

  Especificar o número máximo de peers para cada torrent.  ``0`` significa
  ilimitado.  Ver também a opção :option: `--bt-request-peer-speed-limit`.
  Padrão: ``55``

.. option:: --bt-metadata-only [true|false]

  Download somente os metadados. O(s) arquivo(s) descrito(s) no(s) metadado(s)
  não será(ão) baixado(s). Esta opção só tem efeito para URI BitTorrent Magnet.
  Ver também a opção :option: `--bt-save-metadata`.
  Padrão: ``false``

.. option:: --bt-min-crypto-level=plain|arc4

  Define o nível mínimo do método de critografia.
  Se existem diversos métodos são fornecidos por um peer, aria2 escolhe o que
  satisfaz o menor nível especificado.
  Padrão: ``plain``

.. option:: --bt-prioritize-piece=head[=<TAMANHO>],tail[=<TAMANHO>]

  Tentar primeiramente o download do primeiro e último pedaço de cada arquivo
  Isto é útil para ver antecipadamente os arquivos. O argumento pode conter
  duas palavras chave:
  ``head`` e ``tail``. Para incluir ambos, devem estar separados por vírgula.
  Estas palavras chave possuem um parâmetro tamanho. Por examplo,
  se ``head=<TAMANHO>`` é especificado, pedaço no intervalo do número de bytes
  iniciais de cada arquivo terão prioridade.    ``tail=<TAMANHO>`` significa
  que o intervalo final no TAMANHO informado de cada arquivo. TAMANHO pode
  incluir ``K`` ou ``M`` (1K = 1024, 1M = 1024K). Se TAMANHO é omitido,
  TAMNHA de 1M será usado.

.. option:: --bt-remove-unselected-file [true|false]

   Remove os arquivos não selecionados quando o download do BitTorrent estiver
   completo. Para selecionar arquivo(s) use a opção :option:`--select-file`.
   Se não for usada esta opção, é assumido que todos os arquivos serão 
   selecionados. Use esta opção com critério pois ela realmente remove 
   arquivo(s) do seu disco.
   Padrão: ``false``

.. option:: --bt-require-crypto [true|false]

  Se true é informado, aria2 não aceita nem estabelece conexão com handshake de
  BitTorrent (protocolo \19BitTorrent). Em vez disso aria2 usa 
  (Obfuscation handshake.
  Padrão: ``false`` 

.. option:: --bt-request-peer-speed-limit=<VELOCIDADE>

  Se a velocidade total de download do torrent é menor que a <VELOCIDADE>,
  aria2 temporariamente incrementa o número de peers para tentar maior
  velocidade de download.  Configurando esta opção com sua velocidade 
  preferida pode incrementar a velocidade de download em alguns casos. Pode
  ser anexado ``K`` ou ``M`` (1K = 1024, 1M = 1024K).
  Padrão: ``50K``

.. option:: --bt-save-metadata [true|false]

  Salvar metadados como arquivo ".torrent" file. Esta opção tem efeito somente
  se URI usada é de BitTorrent Magnet.  O nome do arquivo é codificado em hash
  em hexadecimal com sufixo de ".torrent". O diretório onde será salvo o
  o metadado, é o mesmo onde aponta o download do arquivo. Se o arquivo já
  existe, o metadado não será salvo. Ver tambémn a opção :option: 
  `--bt-metadata-only`.
  Padrão: ``false``

.. option:: --bt-seed-unverified [true|false]

  Faz Seed do arquivo previamente baixado sem verificar os hashes dos pedaços.
  Padrão: ``false``

.. option:: --bt-stop-timeout=<SEGUNDOS>

  Interrompe o download do BitTorrent se a velocidade do for zero por 
  consecutivos SEGUNDOS. Se ``0`` é informado, esta funcionalidade é 
  desabilitada.  
  Padrão: ``0``

.. option:: --bt-tracker=<URI>[,...]

  Lista URI, separada por vírgulas, dos rastreadores BitTorrent. Estas URIs não
  são afetadas pela opção :option:`--bt-exclude-tracker`, porque elas são
  adicionadas após as URIs da opção :option:`--bt-exclude-tracker` serem
  removidas.

.. option:: --bt-tracker-connect-timeout=<SEGUNDOS>

  Define o tempo de conexão em segundos para estabelecera conexão com o tracker.
  Após a conexão ser estabelecida, esta opção não tem mais efeito e a opção
  :option:`--bt-tracker-timeout` é usada.
  Padrão: ``60``

.. option:: --bt-tracker-interval=<SEGUNDOS>

  Define o intervalo em segundos, entre as requisições ao tracker / rastreador.
  Isso sobrepõe o valor do intervalo e aria2 passa a usá-los e ignorar o
  valor mínimo de resposta do tracker / rastreador. Se ``0`` é definido, aria2
  assume que o intervalo será baseado no tracker / rastreador e o download
  irá prosseguir.
  Padrão: ``0``

.. option:: --bt-tracker-timeout=<SEGUNDOS>

  Define em segundos o intervalo do timeout.
  Padrão: ``60``

.. option:: --dht-entry-point=<SERVIDOR>:<PORTA>

  Define servidor e a porta da rede DHT IPv4.

.. option:: --dht-entry-point6=<SERVIDOR>:<PORTA>

  Define servidor e a porta da rede DHT IPv6.

.. option:: --dht-file-path=<CAMINHO>

  Modifica o caminho (CAMINHO) para o arquivo da tabela de roteamento DHT IPv4.
  Padrão: ``$HOME/.aria2/dht.dat``

.. option:: --dht-file-path6=<PATH>

  Modifica o caminho (CAMINHO) para o arquivo da tabela de roteamento DHT IPv6.
  Padrão: ``$HOME/.aria2/dht6.dat``

.. option:: --dht-listen-addr6=<ADDR>

  Define o endereço para o bind do socket para DHT IPv6.  Deve ser endereço
  global IPv6 do servidor.

.. option:: --dht-listen-port=<PORT>...

  Define portas UDP para ouvir para DHT (IPv4 e IPv6) e rastreador UDP.
  Múltiplas portas podem ser especificadas através do uso de ``,``,
  por exemplo: ``6881,6885``.  Também pode ser usado ``-`` para especificar
  intervalo, exemplo: ``6881-6999``.  Ambos ``,`` and ``-`` podem ser
  usados em conjunto.
  Padrão: ``6881-6999``

  .. note::

    Cerfifique-se que as portas especificadas estão disponíveis para tráfego UDP
    de entrada.

.. option:: --dht-message-timeout=<SEGUNDOS>

  Define timeout em segundos.
  Padrão: ``10``

.. option:: --enable-dht [true|false]

  Habilita funcionalidade DHT IPv4. Tambem habilita suporte a rastreador UDP.
  Se um identificador particular é usado em um torrente, aria2 não usa DHT
  para aquele download, mesmo que ``true`` foi informado.
  Padrão: ``true``

.. option:: --enable-dht6 [true|false]

   Habilita funcionalidade DHT IPv6. Se identificador particular é usado em um
   torrent, aria2 não usa DHT para aquele download mesmo que ``true`` foi  
   informado. Usar opção :option:`--dht-listen-port` para especificar número(s)
   de porta(s) para ser(em) ouvida(s). Ver também opção :option:`
   --dht-listen-addr6`
   Padrão: ``true``

.. option:: --enable-peer-exchange [true|false]

  Habilita extensão Peer Exchange.  Se um indicador particular é usado nesse 
  torrent, essa funcionalidade será desabilitada para o download, mesmo que
  ``true`` foi informado.
  Padrão: ``true``

.. option:: --follow-torrent=true|false|mem

  Se ``true`` ou ``mem`` é especificado, quando um arquivo cujo sufixo é 
  ``.torrent`` ou o tipo de conteúdo é ``application/x-bittorrent`` é baixado,
  aria2 faz o parse como arquivo torrent e executa o download dos arquivos
  mencionados nele.
  Se ``mem`` é especificado, o arquivo torrent não será gravado em disco, apenas
  será mantido em memória.
  Se ``false`` é especificado, a ação acima descrita não será executada.
  Padrão: ``true``

.. option:: -O, --index-out=<INDEX>=<PATH>

  Define o caminho do arquivo com índice=INDEX. O arquivo índice pode ser
  localizado usando-se a opção :option:`--show-files <-S>`. PATH é o caminho
  relativo ao caminho especificado na opção :option:`--dir <-d>`. 
  Esta opção pode ser usada múltiplas vezes. Com esta opção pode-se especificar
  o nome dos arquivos que serão baixados pelo BitTorrent.

.. option:: --listen-port=<PORT>...

  Define o número das portas TCP para download de BitTorrent.
  Multiplas portas são especificadas usando ``,``,  por exemplo: ``6881,6885``.
  Também pode usar ``-`` para especificar intervalos: ``6881-6999``.
  Ambos ``,`` and ``-`` podem ser usados em conjunto: ``6881-6889,6999``.
  Padrão: ``6881-6999``

  .. note::

    Certifique-se que as portas estejam habilitadas para tráfego TCP de entrada.

.. option:: --max-overall-upload-limit=<VELOCIDADE>

  Define a velocidade máxima geral de upload em bytes/seg.  ``0`` significa
  irrestrito.  Pode anexar ``K`` ou ``M`` (1K = 1024, 1M = 1024K).  Para 
  limitar a velocidade de upload por torrent, usar opção
  :option:`--max-upload-limit <-u>`.
  Padrão: ``0``

.. option:: -u, --max-upload-limit=<VELOCIDADE>

  Define a velocidade máxima para cada torrent em bytes/seg.  ``0`` significa
  irrestrito.  Pode anexar ``K`` ou ``M`` (1K = 1024, 1M = 1024K).  Para 
  limitar a velocidade global de upload de torrent, usar opção
  :option:`--max-overall-upload-limit`.
  Padrão: ``0``

.. option:: --peer-id-prefix=<PEER_ID_PREFIX>

  Especifica o prefixo para ID do peer. O ID do peer em um BitTorrent tem o
  tamanho de 20 bytes. Se mais de 20 bytes são especificados, somente os 20
  bytes iniciais serão usados. Se menos de 20 bytes são especificados, dados
  randomicos serão adicionados para completar o tamanho de 20 bytes.
  Padrão: ``aria2/$VERSÃO-``, $VERSÃO é a versão do pacote aria2.

.. option:: --seed-ratio=<RATIO>

  Specify share ratio. Seed completed torrents until share ratio reaches
  RATIO.
  You are strongly encouraged to specify equals or more than ``1.0`` here.
  Specify ``0.0`` if you intend to do seeding regardless of share ratio.
  If :option:`--seed-time` option is specified along with this option, 
  seeding ends when at least one of the conditions is satisfied.
  Padrão: ``1.0``

.. option:: --seed-time=<MINUTES>

  Especificar o tempo de (seeding) em minutos. Ver também a opção
  :option:`--seed-ratio`.

  .. note::
    
    Especificando :option:`--seed-time=0 <--seed-time>` desabilita o (seeding) 
    após o download ter sido completado.

.. option:: -T, --torrent-file=<TORRENT_FILE>

  O caminho para o arquivo ".torrent".  Não é obrigatório usar esta opção pois
  pode ser especificado arquivo ".torrent" sem a opção
  :option:`--torrent-file <-T>`.

Opções Específicas de Metalink
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. option:: --follow-metalink=true|false|mem

  If ``true`` or ``mem`` is specified, when a file whose suffix is ``.meta4`` 
  or ``.metalink`` or content type of ``application/metalink4+xml`` or 
  ``application/metalink+xml`` is downloaded, aria2 parses it as a metalink
  file and downloads files mentioned in it.
  If ``mem`` is specified, a metalink file is not written to the disk, 
  but is just kept in memory.
  If ``false`` is specified, the action mentioned above is not taken.
  Padrão: ``true``

.. option:: --metalink-base-uri=<URI>

  Specify base URI to resolve relative URI in metalink:url and
  metalink:metaurl element in a metalink file stored in local disk. If
  URI points to a directory, URI must end with ``/``.

.. option:: -M, --metalink-file=<METALINK_FILE>

  The file path to ".meta4" and ".metalink" file. Reads input from stdin when 
  ``-`` is specified.  You are not required to use this option because you can
  specify ".metalink" files without :option:`--metalink-file <-M>`.

.. option:: --metalink-language=<LANGUAGE>

  The language of the file to download.

.. option:: --metalink-location=<LOCATION>[,...]

  The location of the preferred server.
  A comma-delimited list of locations is acceptable, for example, ``jp,us``.

.. option:: --metalink-os=<OS>

  The operating system of the file to download.

.. option:: --metalink-version=<VERSION>

  The version of the file to download.

.. option:: --metalink-preferred-protocol=<PROTO>

  Specify preferred protocol.
  The possible values are ``http``, ``https``, ``ftp`` and ``none``.
  Specify ``none`` to disable this feature.
  Padrão: ``none``
 
.. option:: --metalink-enable-unique-protocol [true|false]

  If ``true`` is given and several protocols are available for a mirror in a
  metalink file, aria2 uses one of them.
  Use :option:`--metalink-preferred-protocol` option to specify
  the preference of protocol.
  Padrão: ``true``

Opções específicas de RPC
~~~~~~~~~~~~~~~~~~~~~~~~~

.. option:: --enable-rpc [true|false]

  Enable JSON-RPC/XML-RPC server.  It is strongly recommended to set username
  and password using :option:`--rpc-user` and :option:`--rpc-passwd`
  option. See also :option:`--rpc-listen-port` option.  Padrão: ``false``

.. option:: --pause [true|false]

  Pause o download após adicionado.  Está opção só é efetiva quando
  a opção :option:`--enable-rpc=true <--enable-rpc>` é informada.
  Padrão: ``false``

.. option:: --rpc-allow-origin-all [true|false]

  Adiciona o campo de cabeçalho,  ``Access-Control-Allow-Origin``,
  com o valor ``*`` á resposta RPC.
  Padrão: ``false``

.. option:: --rpc-certificate=<ARQUIVO>

  Usar o certificado no ARQUIVO para servidor RPC. O certificado
  deve estar no formato PEM.  Usar opção :option:`--rpc-private-key`
  para especificar chave particular. Usar a opção
  :option:`--rpc-secure` para habilitar criptografia.
  Usuários de *AppleTLS* precisam antes gerar o certificado próprio
  auto-assinado através do utilitário ``Keychain Access``, por ex:
  usando o assistente e tomando nota da identificação SHA-1 do
  certificado gerado.
  Para executar o aria2c com a opção :option:`--rpc-secure` usar
  `--rpc-certificate=<SHA-1>` e apenas omitir a opção
  :option:`--rpc-private-key`

.. option:: --rpc-listen-all [true|false]

  Listen incoming JSON-RPC/XML-RPC requests on all network interfaces. If false
  is given, listen only on local loopback interface.  Padrão: ``false``

.. option:: --rpc-listen-port=<PORT>

  Specify a port number for JSON-RPC/XML-RPC server to listen to.  Possible
  Values: ``1024`` -``65535`` Padrão: ``6800``

.. option:: --rpc-max-request-size=<TAMANHO>

  Set max size of JSON-RPC/XML-RPC request. If aria2 detects the request is
  more than SIZE bytes, it drops connection. Padrão: ``2M``

.. option:: --rpc-passwd=<PASSWD>

  Set JSON-RPC/XML-RPC password.

.. option:: --rpc-private-key=<FILE>

  Use the private key in FILE for RPC server.  The private key must be
  decrypted and in PEM format. Use :option:`--rpc-secure` option to
  enable encryption. See also :option:`--rpc-certificate` option.

.. option:: --rpc-save-upload-metadata [true|false]

  Save the uploaded torrent or metalink metadata in the directory
  specified by :option:`--dir` option. The filename consists of SHA-1
  hash hex string of metadata plus extension. For torrent, the
  extension is '.torrent'. For metalink, it is '.meta4'.  If false is
  given to this option, the downloads added by
  :func:`aria2.addTorrent` or :func:`aria2.addMetalink` will not be
  saved by :option:`--save-session` option. Default: ``false``

.. option:: --rpc-secure [true|false]

  RPC transport will be encrypted by SSL/TLS.  The RPC clients must
  use https scheme to access the server. For WebSocket client, use wss
  scheme. Use :option:`--rpc-certificate` and
  :option:`--rpc-private-key` options to specify the server
  certificate and private key.

.. option:: --rpc-user=<USUÁRIO>

  Set JSON-RPC/XML-RPC user.

Opções Avançadas
~~~~~~~~~~~~~~~~
.. option:: --allow-overwrite [true|false]

  Reiniciar o download desde o início se o correspondente arquivo de controle
  não existir.  Ver também a opção :option:`--auto-file-renaming`.  
  Padrão: ``false``

.. option:: --allow-piece-length-change [true|false]

  Se ``false`` é informado, aria2 interrompe o download quando o tamanho de um
  pedaço for diferente do especificado no arquivo controle. Se ``true``
  é informado, o download prossegue mas o progresso será perdido.
  Padrão: ``false``

.. option:: --always-resume [true|false]

  Sempre continuar. Se If ``true`` é informado, aria2 sempre tentará
  retomar o download do ponto interrompido e se não for possivel o download
  será interrompido.  Se ``false`` é informado, quando todas URIs fornecidas
  não suportarem a continuidade do download ou aria2 encontrar ``N`` URIs as
  quais não suportem retomar o download (``N`` é o valor especificado na
  opção :option:`--max-resume-failure-tries`), aria2 irá iniciar o download
  do início.  Ver opção :option:`--max-resume-failure-tries`
  Padrão: ``true``

.. option:: --async-dns [true|false]

  Habilita DNS assíncrono.
  Padrão: ``true``

.. option:: --async-dns-server=<ENDEREÇO-IP>[,...]

  Lista separada por vírgulas, dos endereços dos servidores DNS assíncronos
  usados pelo resolvedor. Normalmente o resolvedor de DNS assíncronos faz a
  leitura dos endereços a partir do arquivo ``/etc/resolv.conf``.
  Quando essa opção é usada é feito uso dos servidores DNS especificados na
  opção em detrimento do conteúdo do arquivo ``/etc/resolv.conf``. 
  Podem ser usados ambos endereços IPv4 e IPv6. Essa opção é útil
  quando o sistema não possui ``/etc/resolv.conf`` e o usuário não tem
  permissão para criá-lo.

.. option:: --auto-file-renaming [true|false]

  Renomear o arquivo se o mesmo já existir.
  Essa opção só funciona em download HTTP, HTTPS e FTP.
  O novo nome do arquivo terá um ponto e uma sequência (1..9999) como sufixo.
  Padrão: ``true``

.. option:: --auto-save-interval=<SEGUNDOS>

  Salvar o arquivo de controle (\*.aria2) a cada intervalo de SEGUNDOS.
  Se ``0`` é informado, o arquivo de controle não será salvo durante o
  download. aria2 salva o arquivo de controle quando parar, independentemente
  do valor. As possibilidades vão desde ``0`` até ``600``.
  Padrão: ``60``

.. option:: --conditional-get [true|false]

  Download file only when the local file is older than remote
  file. This function only works with HTTP, HTTPS,  downloads only.  It does
  not work if file size is specified in Metalink. It also ignores
  Content-Disposition header.  If a control file exists, this option
  will be ignored.  This function uses If-Modified-Since header to get
  only newer file conditionally. When getting modification time of
  local file, it uses user supplied filename(see :option:`--out <-o>` option) or
  filename part in URI if :option:`--out <-o>` is not specified.
  To overwrite existing file, :option:`--allow-overwrite` is required.
  Padrão: ``false``

.. option:: --conf-path=<PATH>

  Change the configuration file path to PATH.
  Padrão: ``$HOME/.aria2/aria2.conf``

.. option:: --console-log-level=<LEVEL>

  Set log level to output to console.  LEVEL is either ``debug``,
  ``info``, ``notice``, ``warn`` or ``error``.  Default: ``notice``

.. option:: -D, --daemon [true|false]

  Run as daemon. The current working directory will be changed to ``/``
  and standard input, standard output and standard error will be
  redirected to ``/dev/null``. Padrão: ``false``

.. option:: --deferred-input [true|false]

  If ``true`` is given, aria2 does not read all URIs and options from file
  specified by :option:`--input-file <-i>` option at startup,
  but it reads one by one when it
  needs later. This may reduce memory usage if input file contains a
  lot of URIs to download.  If ``false`` is given, aria2 reads all URIs
  and options at startup.
  Padrão: ``false``

.. option:: --disable-ipv6 [true|false]

  Disable IPv6. This is useful if you have to use broken DNS and want
  to avoid terribly slow AAAA record lookup. Padrão: ``false``

.. option:: --disk-cache=<SIZE>

  Enable disk cache. If SIZE is ``0``, the disk cache is
  disabled. This feature caches the downloaded data in memory, which
  grows to at most SIZE bytes. The cache storage is created for aria2
  instance and shared by all downloads. The one advantage of the disk
  cache is reduce the disk I/O because the data are written in larger
  unit and it is reordered by the offset of the file.  If hash
  checking is involved and the data are cached in memory, we don't
  need to read them from the disk.  SIZE can include ``K`` or ``M``
  (1K = 1024, 1M = 1024K). Default: ``16M``

.. option:: --download-result=<OPT>

  This option changes the way ``Download Results`` is formatted. If OPT
  is ``default``, print GID, status, average download speed and
  path/URI. If multiple files are involved, path/URI of first
  requested file is printed and remaining ones are omitted.  If OPT is
  ``full``, print GID, status, average download speed, percentage of
  progress and path/URI. The percentage of progress and path/URI are
  printed for each requested file in each row.
  Padrão: ``default``

.. option:: --enable-async-dns6 [true|false]

  Enable IPv6 name resolution in asynchronous DNS resolver. This
  option will be ignored when :option:`--async-dns=false. <--async-dns>`
  Padrão: ``false``

.. option:: --enable-mmap [true|false]

   Map files into memory. This option may not work if the file space
   is not pre-allocated. See :option:`--file-allocation`.

   Padrão: ``false``

.. option:: --event-poll=<POLL>

  Specify the method for polling events.  The possible values are
  ``epoll``, ``kqueue``, ``port``, ``poll`` and ``select``.  For each ``epoll``,
  ``kqueue``, ``port`` and ``poll``, it is available if system supports it.
  ``epoll`` is available on recent Linux. ``kqueue`` is available on
  various \*BSD systems including Mac OS X. ``port`` is available on Open
  Solaris. The default value may vary depending on the system you use.

.. option:: --file-allocation=<METHOD>

  Specify file allocation method.
  ``none`` doesn't pre-allocate file space. ``prealloc`` pre-allocates file space
  before download begins. This may take some time depending on the size of the
  file.
  If you are using newer file systems such as ext4
  (with extents support), btrfs, xfs or NTFS(MinGW build only), ``falloc`` is
  your best choice. It allocates large(few GiB)
  files almost instantly. Don't use ``falloc`` with
  legacy file systems such as ext3 and FAT32 because it takes
  almost same time as ``prealloc`` and it blocks aria2
  entirely until allocation finishes. ``falloc`` may
  not be available if your system doesn't have
  :manpage:`posix_fallocate(3)` function.
  ``trunc`` uses :manpage:`ftruncate(2)` system call or
  platform-specific counterpart to truncate a file to a specified
  length.

  Possible Values: ``none``, ``prealloc``, ``trunc``, ``falloc``
  Padrão: ``prealloc``

.. option:: --force-save [true|false]

  Save download with :option:`--save-session <--save-session>` option
  even if the download is completed or removed. This may be useful to
  save BitTorrent seeding which is recognized as completed state.
  Default: ``false``

.. option:: --gid=<GID>

  Set GID manually. aria2 identifies each download by the ID called
  GID. The GID must be hex string of 16 characters, thus [0-9a-zA-Z]
  are allowed and leading zeros must not be stripped. The GID all 0 is
  reserved and must not be used. The GID must be unique, otherwise
  error is reported and the download is not added.  This option is
  useful when restoring the sessions saved using
  :option:`--save-session <--save-session>` option. If this option is
  not used, new GID is generated by aria2.

.. option:: --hash-check-only [true|false]

  If ``true`` is given, after hash check using
  :option:`--check-integrity <-V>` option,
  abort download whether or not download is complete.
  Padrão: ``false``

.. option:: --human-readable [true|false]

  Print sizes and speed in human readable format (e.g., 1.2Ki, 3.4Mi)
  in the console readout. Padrão: ``true``

.. option:: --interface=<INTERFACE>

  Bind sockets to given interface. You can specify interface name, IP
  address and hostname.
  Possible Values: interface, IP address, hostname

  .. note::

    If an interface has multiple addresses, it is highly recommended to
    specify IP address explicitly. See also :option:`--disable-ipv6`.  If your
    system doesn't have :manpage:`getifaddrs(3)`, this option doesn't accept interface
    name.

.. option:: --max-download-result=<NÚMERO>

  Set maximum number of download result kept in memory. The download
  results are completed/error/removed downloads. The download results
  are stored in FIFO queue and it can store at most NUM download
  results. When queue is full and new download result is created,
  oldest download result is removed from the front of the queue and
  new one is pushed to the back. Setting big number in this option may
  result high memory consumption after thousands of
  downloads. Specifying 0 means no download result is kept. Padrão:
  ``1000``

.. option:: --max-resume-failure-tries=<N>

  When used with :option:`--always-resume=false, <--always-resume>` aria2 downloads file from
  scratch when aria2 detects N number of URIs that does not support
  resume. If N is ``0``, aria2 downloads file from scratch when all
  given URIs do not support resume.  See :option:`--always-resume` option.
  Padrão: ``0``

.. option:: --log-level=<LEVEL>

  Set log level to output.
  LEVEL is either ``debug``, ``info``, ``notice``, ``warn`` or ``error``.
  Padrão: ``debug``

.. option:: --on-bt-download-complete=<COMMAND>

  For BitTorrent, a command specified in :option:`--on-download-complete` is
  called after download completed and seeding is over. On the other
  hand, this option set the command to be executed after download
  completed but before seeding.
  See `Interação com Eventos (Hook)`_ for more details about COMMAND.
  Possible Values: ``/path/to/command``

.. option:: --on-download-complete=<COMMAND>

  Set the command to be executed after download completed.  See
  See `Interação com Eventos (Hook)`_ for more details about COMMAND.
  See also :option:`--on-download-stop` option.
  Possible Values: ``/path/to/command``

.. option:: --on-download-error=<COMMAND>

  Set the command to be executed after download aborted due to error.
  See `Interação com Eventos (Hook)`_ for more details about COMMAND.
  See also :option:`--on-download-stop` option.  Possible Values:
  ``/path/to/command``

.. option:: --on-download-pause=<COMMAND>

  Set the command to be executed after download was paused.
  See `Interação com Eventos (Hook)`_ for more details about COMMAND.
  Possible Values: ``/path/to/command``

.. option:: --on-download-start=<COMMAND>

  Set the command to be executed after download got started.
  See `Interação com Eventos (Hook)`_ for more details about COMMAND.
  Possible Values: ``/path/to/command``

.. option:: --on-download-stop=<COMMAND>

  Set the command to be executed after download stopped. You can override
  the command to be executed for particular download result using
  :option:`--on-download-complete` and :option:`--on-download-error`. If they are
  specified, command specified in this option is not executed.
  See `Interação com Eventos (Hook)`_ for more details about COMMAND.
  Possible Values: ``/path/to/command``

.. option:: --piece-length=<LENGTH>

  Set a piece length for HTTP e FTP downloads. This is the boundary when
  aria2 splits a file. All splits occur at multiple of this
  length. This option will be ignored in BitTorrent downloads.  It
  will be also ignored if Metalink file contains piece hashes.
  Padrão: ``1M``

  .. note::
    
    The possible usecase of :option:`--piece-length`
    option is change the request range in one HTTP pipelined request.
    To enable HTTP pipelining use
    :option:`--enable-http-pipelining`.

.. option:: --show-console-readout [true|false]

  Show console readout. Padrão: ``true``

.. option:: --summary-interval=<SEGUNDOS>

  Set interval in seconds to output download progress summary.
  Setting ``0`` suppresses the output.
  Padrão: ``60``

  .. note::

    In multi file torrent downloads, the files adjacent forward to the specified files
    are also allocated if they share the same piece.

.. option:: -Z, --force-sequential [true|false]

  Fetch URIs in the command-line sequentially and download each URI in a
  separate session, like the usual command-line download utilities.
  Padrão: ``false``

.. option:: --max-overall-download-limit=<VELOCIDADE>

  Set max overall download speed in bytes/sec.  ``0`` means
  unrestricted.  You can append ``K`` or ``M`` (1K = 1024, 1M = 1024K).  To
  limit the download speed per download, use :option:`--max-download-limit`
  option.  Padrão: ``0``

.. option:: --max-download-limit=<VELOCIDADE>

  Set max download speed per each download in bytes/sec. ``0`` means
  unrestricted.  You can append ``K`` or ``M`` (1K = 1024, 1M = 1024K).  To
  limit the overall download speed, use :option:`--max-overall-download-limit`
  option.  Padrão: ``0``

.. option:: --no-conf [true|false]

  Disable loading aria2.conf file.

.. option:: --no-file-allocation-limit=<TAMANHO>

  No file allocation is made for files whose size is smaller than SIZE.
  You can append ``K`` or ``M`` (1K = 1024, 1M = 1024K).
  Padrão: ``5M``

.. option:: -P, --parameterized-uri [true|false]

  Enable parameterized URI support.
  You can specify set of parts: ``http://{sv1,sv2,sv3}/foo.iso``.
  Also you can specify numeric sequences with step counter:
  ``http://host/image[000-100:2].img``.
  A step counter can be omitted.
  If all URIs do not point to the same file, such as the second example above,
  -Z option is required.
  Padrão: ``false``

.. option:: -q, --quiet [true|false]

  Make aria2 quiet (no console output).
  Padrão: ``false``

.. option:: --realtime-chunk-checksum [true|false]

   Validate chunk of data by calculating checksum while downloading a file if
   chunk checksums are provided.
   Padrão: ``true``


.. option:: --remove-control-file [true|false]

   Remove control file before download. Using with
   :option:`--allow-overwrite=true, <--allow-overwrite>` download always starts from
   scratch. This will be useful for users behind proxy server which
   disables resume.

.. option:: --save-session=<ARQUIVO>

  Salvar downloads não concluídos ou com erro, para um ARQUIVO quando sair.
  Pode ser informado o nome do arquivo para o aria2 com a opção
  :option:`--input-file <-i>` no restart.  Note que downloads adicionados
  pela função :func:`aria2.addTorrent` e pela função
  :func:`aria2.addMetalink` método RPC e seus respectivos metadados não podem
  ser salvos. Downloads removidos usando a função :func:`aria2.remove` e
  :func:`aria2.forceRemove` não serão salvos.

.. option:: --save-session-interval=<SEC>

  Save error/unfinished downloads to a file specified by
  :option:`--save-session` option every SEC seconds. If ``0`` is
  given, file will be saved only when aria2 exits. Default: ``0``

.. option:: --stop=<SEGUNDOS>

  Finaliza a aplicação após SEGUNDOS se passarem.
  Se ``0`` é informado, essa funcionalidade é desabilitada.
  Padrão: ``0``

.. option:: --stop-with-process=<PID>

  Finaliza a aplicação quando o processo de número PID não estiver executando.
  Isso é útil se o processo aria2 foi derivado de um processo precursor.
  O processo precursor pode ter criado o processo aria2 com seu próprio pid
  e quando o processo precursor terminar por alguma razão, aria2 pode
  detectar por ele mesmo, essa situação e executar ele mesmo o shutdown.
  Este recurso evita que um processo gere subprocessos que ficam no
  limbo.

.. option:: --truncate-console-readout [true|false]

  Truncar a linha da console para ajustar-se a uma linha.
  Padrão: ``true``
 
.. option:: -v, --version

  Exibe o número da versão, copyright e informação da configuração e sai.

Argumento(s) Opcional(is)
~~~~~~~~~~~~~~~~~~~~~~~~~
As opções que possuem seus argumentos delimitados por colchetes ([]), são
opcionais. Normalmente omitindo o argumento, ele será assumido como ``true``
(verdadeiro).
Se for usada a forma abreviada dessas opções (como em ``-V``) e informado
um argumento, estão o nome da opção e seu argumento devem ser concatenados,
por exemplo: (``-Vfalse``). Se houver espaços entre a opção e o argumento o
argumento será tratado como URI e normalmente não é isto o esperado.

Unidades (K and M)
^^^^^^^^^^^^^^^^^^

Algumas opções usam ``K`` e ``M`` para convenientemente representar
1.024 e 1.048.576 respectivamente.  aria2 detecta estas caracteres de maneira
transparente (maiúsculas e minúsculas), portanto podem ser usados
`k`` ou ``K`` e ``m`` ou ``M``.

URI, MAGNET, TORRENT_FILE, METALINK_FILE
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Podemos especificar múltiplas URIs em uma linha de comando.  A menos que seja
especificada a opção :option:`--force-sequential <-Z>`, todas as URIs devem
apontar para o mesmo arquivo que será baixado ou o download falhará.

Pode-se especificar um número arbitrátio de URIs de BitTorrent Magnet URI.
Nove que eles sempre serão tratados como downloads separados.
São suportados Hash de Info de 40 characters  e Hast de Info Base32 de 32 
characters. Múltiplos parâmetros ``tr`` são suportados.  Devido a URI
BitTorrent Magnet poder conter o caracter ``&``, é altamente recomendável
delimitar a URI com apóstrofo single(``'``) ou aspas double(``"``).
É altamente recomendável habilitar DHT especialmente quando ``tr`` o parâmetro
estiver ausente.  Ver http://www.bittorrent.org/beps/bep_0009.html
para maiores detalhes sobre URI BitTorrent Magnet.

Pode-se também especificar um número arbitrário de arquivos torrent e 
Documentos Metalink armazenados em um dispositivo local. Note que sempre serão
tratados como download distintos. Tanto Metalink4 quanto Metalink da versão 3
são suportados.

Pode-se especificar arquivo torrent usando a opção -T e URI. Fazendo isso
o download será baixado do servidor swarm e HTTP, HTTPS e FTP ao mesmo tempo,
enquanto os dados do HTTP, HTTPS e FTP serão uploaded para o swarm torrent.
Para torrent de um arquivo a URI deve ser completa e apontar inclusive o 
recurso ou se a URI terminar com / o nome do torrent será adicionado. Para
múltiplos torrents, name e caminho serão adicionados para formar a URI, para
cada um dos arquivos.

.. note::

  Certifique-se que a URI seja delimitada por apóstrofo single(``'``) ou 
  aspas double(``"``) se a URI contiver ``&`` ou qualquer outro caracter que
  tenha significado especial para o shell.

Continuar Download Interrompido
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Normalmente pode-se retomar uma transferência apenas executando-se o comando
aria2c URI, caso a transferência anterior estava sendo feita pelo aria2.

Caso a transferência anterior estava sendo feita por um navegador ou wget de
maneira sequencial, então utilize a opção :option:`--continue <-c>` para
retomar do ponto onde foi interrompida a transferência.

Interação com Eventos (Hook)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

aria2 possui opções para especificar comando arbitrário após um evento 
específico ocorrer. Atualmente as seguintes opções estão disponíveis:

:option:`--on-bt-download-complete`,
:option:`--on-download-pause`,
:option:`--on-download-complete`.
:option:`--on-download-start`,
:option:`--on-download-error`,
:option:`--on-download-stop`.

aria2 passa 3 argumentos para um comando especificado quando este comando for
executado. Estes argumentos são: GID, o número de arquivos e o caminho dos
arquivos.  Para downloads HTTP, HTTPS e FTP normalmente o número de arquivos é 1.
BitTorrent podem conter múltiplos arquivos. Se o número de arquivos é maior
que 1, o caminho do arquivo é o primeiro.  Em outras palavras, este é o valor
da chave path da primeira estrutura se aquela chave for verdadeira como 
resposta do método da função :func:`aria2.getFiles` RPC.
Se for necessário todos os caminhos dos arquivos, considere usar o método
JSON-RPC/XML-RPC.  Lembre-se que o caminho do arquivo pode ser modificado 
durante o download no HTTP por causa do redirecionamento ou Disposição do
Conteúdo do Cabeçalho.

Observemos um exemplo de como são passados argumentos para um comando:

.. code-block:: console

  $ cat hook.sh
  #!/bin/sh
  echo "chamado com [$1] [$2] [$3]"
  $ aria2c --on-download-complete hook.sh http://dobrasil.org/arquivo.iso
  chamado com [1] [1] [/caminho/para/arquivo.iso]


CÓDIGO DE RETORNO ou SAÍDA
--------------------------

Como aria2 pode manipular múltiplos downloads concomitantemente, podem ser
encontrados vários erros durante uma sessão.  aria2 retorna os códigos de
retorno ou saída com base no último erro encontrado.

0
  Se todos os downloads terminam com sucesso.

1
  Erro desconhecido ocorreu.

2
  Tempo transcorrido (time out).

3
  Recurso não encontrado.

4
  Se aria2 tentou um número especificado de vezes e obteve "recurso não encontrado".
  Ver opção :option:`--max-file-not-found`.

5
  Se download interrompido porque a velocidade de download é muito baixa.
  Ver opção :option:`--lowest-speed-limit`

6
  Erro na rede (network).

7
  Se existem downloads não concluidos.  Esse erro é reportado somente se
  todos os downloads foram concluídos com sucesso mas existe uma fila quando
  aria2 foi interrompido por quando foi pressionado :kbd:`Ctrl-C` pelo usuário
  ou enviado o sinal TERM ou INT.

8
  Se o servidor remoto não suporta retomar um download que deve ser completado.

9
  Não há espaço em disco suficiente para os downloads.

10
  Se o tamanho do pedaço (piece) foi diferente do arquivo de controle .aria2.
  Ver opção :option:`--allow-piece-length-change`.

11
  Se aria2 estava fazendo o download do mesmo arquivo no momento.

12
  If aria2 estava fazendo o download do mesmo, hash do torrent, no momento.

13
  Se o arquivo já existe. Ver opção :option:`--allow-overwrite`.

14
  Se renomer o arquivo falhou. Ver opção :option:`--auto-file-renaming`.

15
  Se aria2 não pode abrir o arquivo existente.

16
  Se aria2 não pode criar novo arquivo ou truncar um arquivo já existente.

17
  Se ocorreu erro de I/O no arquivo.

18
  Se aria2 não pode criar diretório.

19
  Se resolução de nomes falhou.

20
  Se aria2 não pode passar documento Metalink.

21
  Se comando FTP falhou.

22
  Se cabeçalho de resposta HTTP está errado ou inesperado.

23
  Se muitos redirecionamentos ocorreram.

24
  Se autorização HTTP falhou.

25
  Se aria2 não pode passar arquivo bencoded file (normalmente arq.  ".torrent").

26
  Se arquivo ".torrent" estava corrompido ou inexistem informações que aria2
  necessita.

27
  Se URI Magnet URI está errada.

28
  Se opção está errada ou não é reconhecida ou argumento inválido de uma opção 
  foi informado.

29
  Se o servidor remoto não pode manusear uma requisição devido a sobrecarga 
  temporária ou manutenção.

30
  Se aria2 não pode passar uma requisição JSON-RPC.

.. note::

  Um erro ocorrido em um download completado não será reportado como um 
  status de saida nem como código de retorno (exit status).

.. index:: double: Variáveis; Ambiente

VARIÁVEIS DE AMBIENTE
---------------------
aria2 reconhece as seguintes variáveis de ambiente.

``http_proxy [http://][USUÁRIO:SENHA@]SERVIDOR[:PORTA]``
  Especifica o servidor para uso do HTTP.
  Sobrepõe o valor do http-proxy do arquivo de configuração.
  A opção linha de comando :option:`--http-proxy` sobrepõe este valor.

``https_proxy [http://][USUÁRIO:SENHA@]SERVIDOR[:PORTA]``
  Especifica o servidor proxy para uso do HTTPS.
  Sobrepõe o valor do https-proxy do arquivo de configuração.
  A opção linha de comando :option:`--https-proxy` sobrepõe este valor.

``ftp_proxy [http://][USUÁRIO:SENHA@]SERVIDOR[:PORTA]``
  Especifica o servidor proxy para uso do FTP.
  Sobrepõe o valor do http-proxy do arquivo de configuração.
  A opção linha de comando :option:`--ftp-proxy` sobrepõe este valor.

``all_proxy [http://][USUÁRIO:SENHA@]SERVIDOR[:PORTA]``
  Especifica o servidor proxy para uso se nenhum protocolo-específico de proxy
  é informado.
  Sobrepõe o valor all-proxy do arquivo de configuração.
  A opção linha de comando :option:`--all-proxy` sobrepõe este valor.

.. note::
  
  Apesar de aria2 aceitar os esquemas ``ftp://`` e ``https://`` para proxy de
  URI, aria2 assume que ``http://`` é especificado e não troca o comportamento
  com base no esquema especificado.

``no_proxy [DOMÍNIO,...]``
  Especifica nome de servidores, separados por vírgula, domínios e endereços 
  de rede com ou sem blocos CIDR para os quais não será usado proxy.
  Sobrepõe o valor no-proxy do arquivo de configuração.
  A opção linha de comando :option:`--no-proxy` sobrepõe este valor.

Arquivos de configuração
------------------------

.. index:: triple:   aria2.conf; arquivo; configuração

aria2.conf
~~~~~~~~~~

Por padrão, aria2 utiliza ``$HOME/.aria2/aria2.conf`` como arquivo de
configuração. Pode ser especificado o caminho do arquivo usando-se a opção
:option:`--conf-path`.  Se não desejar usar a configuração de arquivo utilize
a opção :option:`--no-conf`.

O arquivo de configuração é um arquivo texto e possui uma opção por linha.
Em cada linha, deve haver um par (nome-valor), no formato:
``NOME=VALOR``, onde nome é o nome longo da linha de comando sem o prefixo
``--``. Pode ser usada a mesma sintaxe da opção da linha de comando.
Linhas que começam com ``#`` são tratados como comentários::

  # arquivo de simples configuração para aria2c
  listen-port=60000
  dht-listen-port=60000
  seed-ratio=1.0
  max-upload-limit=50K
  ftp-pasv=true

.. note::

  Informações confidenciais como usuário/senha podem ser incluídas no arquivo
  de configuração, mas recomenda-se trocar os bits de seu modo de acesso
  (por exemplo: ``chmod 600 aria2.conf``), dessa maneira nenhum outro usuário
  consegue ver o conteúdo desse arquivo de configuração.

.. index:: triple:   dht.dat; arquivo; configuração

dht.dat
~~~~~~~

Por padrão, a tabela de rota do IPv4 DHT está em ``$HOME/.aria2/dht.dat`` e a
tabela de rota do IPv6 DHT está em ``$HOME/.aria2/dht6.dat``.

.. index:: triple:   netrc; arquivo; configuração

Netrc
~~~~~

Por padrão, o suporte a Netrc está habilitado para HTTP, HTTPS e FTP.  Para 
desabilitar o suporte a netrc especificar :option:`--no-netrc <-n>`.  Seu 
arquivo .netrc precisa ter as permissões corretas (600).

Se o nome do servidor inicia com ``.``, aria2 executa pesquisa domínio ao 
invés de pesquisa exata. Isto é uma extensão de aria2. Exemplo de pesquisa
de domínio, imagine as seguinte entrada no arquivo .netrc::

  machine .dobrasil.org login meuid password minhasenha


``aria2.dobrasil.org`` pesquisa domínio ``.dobrasil.org`` e usa ``meuid`` e
``minhasenha``.

Mais exemplos de pesquisa domínio: ``nobrasil.net`` não executa pesquisa
domínio ``.nobrasil.net``. ``dobrasil.org`` não faz pesquisa domínio ``.dobrasil.org``
pois tem maior amplitude hierárquica por causa do ponto precedente ``.``.  Se 
desejar utilizar ``dobrasil.org``, especificar ``dobrasil.org``.

.. index:: triple:   aria2; arquivo; controle

Arquivo de Controle
~~~~~~~~~~~~~~~~~~~

aria2 utiliza um arquivo de controle para rastrear o progresso
de um download.  Este arquivo é gravado no mesmo diretório do
arquivo que está sendo baixado e possui o sufixo ``.aria2``.
Por exemplo,se está baixando arquivo.zip, então o arquivo de controle
será arquivo.zip.aria2.  (Existe apenas uma exceção para essa convenção
que é quando você está baixando um multi torrent, o arquivo de controle
estará no "diretório topo" do torrent com o sufixo ``.aria2``.  O nome do
"diretório topo" é o valor da chave "name" no diretório "info" do arquivo
torrent).

Normalmente um arquivo controle é apagado quando o download se completa.  Se
aria2 detecta que o download não pode ser retomado (por exemplo, quando faz 
download de um servidor HTTP que não suporta retomar o processamento de um
ponto mas sempre do início), o arquivo de controle não será criado.

Se você perder o arquivo de controle, não será possivel retomar o download do
ponto onde estava. Mas se há um torrent ou metalink com checksum (verificação) do
arquivo, poderá ser retomado do último ponto especificando a opção -V na linha
de comando.

.. index:: triple:   aria2; arquivo; entrada

Arquivo de Entrada
~~~~~~~~~~~~~~~~~~

O arquivo de entrada pode conter uma lista de URIs para que o aria2 efetua
download.  Podem ser especificados múltiplas URIs para cada simples entidade:
separar as entidades por um caracter TAB ou espaço.

Cada linha é tratada como se fosse especificada através de um argumento da linha
de comando. Entretando estas linhas são afetadas pela opção 
:option:`--force-sequential <-Z>` e pela opção :option:`--parameterized-uri <-P>`

Quando as URIs do arquivo de entrada são diretamente lidas pelo aria2, as URIs
não precisam necessáriamente estarem delimitadas por apóstrofo (``'``) nem
aspas(``"``).

Linhas que começam com ``#`` são tratadas como comentário e desprezadas.

Adicionalmente, as seguintes opções podem ser especificadas após cada linha de
URI. Estas linhas opcionais precisam iniciar com um ou mais espaços.


.. hlist::
  :columns: 3
 
  * :option:`all-proxy <--all-proxy>`
  * :option:`all-proxy-passwd <--all-proxy-passwd>`
  * :option:`all-proxy-user <--all-proxy-user>`
  * :option:`allow-overwrite <--allow-overwrite>`
  * :option:`allow-piece-length-change <--allow-piece-length-change>`
  * :option:`always-resume <--always-resume>`
  * :option:`async-dns <--async-dns>`
  * :option:`auto-file-renaming <--auto-file-renaming>`
  * :option:`bt-enable-lpd <--bt-enable-lpd>`
  * :option:`bt-exclude-tracker <--bt-exclude-tracker>`
  * :option:`bt-external-ip <--bt-external-ip>`
  * :option:`bt-hash-check-seed <--bt-hash-check-seed>`
  * :option:`bt-max-open-files <--bt-max-open-files>`
  * :option:`bt-max-peers <--bt-max-peers>`
  * :option:`bt-metadata-only <--bt-metadata-only>`
  * :option:`bt-min-crypto-level <--bt-min-crypto-level>`
  * :option:`bt-prioritize-piece <--bt-prioritize-piece>`
  * :option:`bt-remove-unselected-file <--bt-remove-unselected-file>`
  * :option:`bt-request-peer-speed-limit <--bt-request-peer-speed-limit>`
  * :option:`bt-require-crypto <--bt-require-crypto>`
  * :option:`bt-save-metadata <--bt-save-metadata>`
  * :option:`bt-seed-unverified <--bt-seed-unverified>`
  * :option:`bt-stop-timeout <--bt-stop-timeout>`
  * :option:`bt-tracker <--bt-tracker>`
  * :option:`bt-tracker-connect-timeout <--bt-tracker-connect-timeout>`
  * :option:`bt-tracker-interval <--bt-tracker-interval>`
  * :option:`bt-tracker-timeout <--bt-tracker-timeout>`
  * :option:`check-integrity <-V>`
  * :option:`checksum <--checksum>`
  * :option:`conditional-get <--conditional-get>`
  * :option:`connect-timeout <--connect-timeout>`
  * :option:`continue <-c>`
  * :option:`dir <-d>`
  * :option:`dry-run <--dry-run>`
  * :option:`enable-async-dns6 <--enable-async-dns6>`
  * :option:`enable-http-keep-alive <--enable-http-keep-alive>`
  * :option:`enable-http-pipelining <--enable-http-pipelining>`
  * :option:`enable-mmap <--enable-mmap>`
  * :option:`enable-peer-exchange <--enable-peer-exchange>`
  * :option:`file-allocation <--file-allocation>`
  * :option:`follow-metalink <--follow-metalink>`
  * :option:`follow-torrent <--follow-torrent>`
  * :option:`force-save <--force-save>`
  * :option:`ftp-passwd <--ftp-passwd>`
  * :option:`ftp-pasv <-p>`
  * :option:`ftp-proxy <--ftp-proxy>`
  * :option:`ftp-proxy-passwd <--ftp-proxy-passwd>`
  * :option:`ftp-proxy-user <--ftp-proxy-user>`
  * :option:`ftp-reuse-connection <--ftp-reuse-connection>`
  * :option:`ftp-type <--ftp-type>`
  * :option:`ftp-user <--ftp-user>`
  * :option:`hash-check-only <--hash-check-only>`
  * :option:`header <--header>`
  * :option:`http-accept-gzip <--http-accept-gzip>`
  * :option:`http-auth-challenge <--http-auth-challenge>`
  * :option:`http-no-cache <--http-no-cache>`
  * :option:`http-passwd <--http-passwd>`
  * :option:`http-proxy <--http-proxy>`
  * :option:`http-proxy-passwd <--http-proxy-passwd>`
  * :option:`http-proxy-user <--http-proxy-user>`
  * :option:`http-user <--http-user>`
  * :option:`https-proxy <--https-proxy>`
  * :option:`https-proxy-passwd <--https-proxy-passwd>`
  * :option:`https-proxy-user <--https-proxy-user>`
  * :option:`index-out <-O>`
  * :option:`lowest-speed-limit <--lowest-speed-limit>`
  * :option:`max-connection-per-server <-x>`
  * :option:`max-download-limit <--max-download-limit>`
  * :option:`max-file-not-found <--max-file-not-found>`
  * :option:`max-resume-failure-tries <--max-resume-failure-tries>`
  * :option:`max-tries <-m>`
  * :option:`max-upload-limit <-u>`
  * :option:`metalink-base-uri <--metalink-base-uri>`
  * :option:`metalink-enable-unique-protocol <--metalink-enable-unique-protocol>`
  * :option:`metalink-language <--metalink-language>`
  * :option:`metalink-location <--metalink-location>`
  * :option:`metalink-os <--metalink-os>`
  * :option:`metalink-preferred-protocol <--metalink-preferred-protocol>`
  * :option:`metalink-version <--metalink-version>`
  * :option:`min-split-size <-k>`
  * :option:`no-file-allocation-limit <--no-file-allocation-limit>`
  * :option:`no-netrc <-n>`
  * :option:`no-proxy <--no-proxy>`
  * :option:`out <-o>`
  * :option:`parameterized-uri <-P>`
  * :option:`pause <--pause>`
  * :option:`piece-length <--piece-length>`
  * :option:`proxy-method <--proxy-method>`
  * :option:`realtime-chunk-checksum <--realtime-chunk-checksum>`
  * :option:`referer <--referer>`
  * :option:`remote-time <-R>`
  * :option:`remove-control-file <--remove-control-file>`
  * :option:`retry-wait <--retry-wait>`
  * :option:`reuse-uri <--reuse-uri>`
  * :option:`rpc-save-upload-metadata <--rpc-save-upload-metadata>`
  * :option:`seed-ratio <--seed-ratio>`
  * :option:`seed-time <--seed-time>`
  * :option:`select-file <--select-file>`
  * :option:`split <-s>`
  * :option:`stream-piece-selector <--stream-piece-selector>`
  * :option:`timeout <-t>`
  * :option:`uri-selector <--uri-selector>`
  * :option:`use-head <--use-head>`
  * :option:`user-agent <-U>`
  
Estas opções possuem exatamente o mesmo significado das opções existentes na
linha de comando, mas aplicam-se apenas a URI a que pertencem.
Por favor perceba que dentro de um arquivo, elas não terão o prefixo ``--``.

Por exemplo, o conteúdo do arquivo de entrada uri.txt é::

  http://servidor/arquivo.iso http://espelho/arquivo.iso
    dir=/imagens_iso
    out=arquivo.img
  http://fu/ba


Se aria2 é executado com as opções ``-i uri.txt -d /tmp``, então o
``arquivo.iso`` será salvo como ``/imagens_iso/arquivo.img`` e será baixado
dos servidores ``http://servidor/arquivo.iso`` e  
``http://espelho/arquivo.iso``.
O arquivo ``ba`` e baixado de ``http://fu/ba`` e salvo como ``/tmp/ba``.

Em alguns casos, o parâmetro :option:`out <-o>` não tem efeito.
Ver nota da opção :option:`--out <-o>` para entender as restrições.

.. index:: triple: Servidor; performance; Perfil;

Perfil Performance Servidor
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Esta seção descreve o formato do perfil de performance do servidor, composto
por um arquivo de texto plano com cada linha contendo um par ``NOME=VALOR``,
delimitados por virgula. Atualmente esta é a lista de nomes reconhecidos:

``host``
  Nome do servidor. Requerido.

``protocol``
  Protocolo para este perfil, como ftp, http, https. http é requerido.

``dl_speed``
  Velocidade média de download observada no download prévio, em bytes por 
  segundo.  Requerido.

``sc_avg_speed``
  The average download speed observed in the previous download in
  bytes per sec. This value is only updated if the download is done in
  single connection environment and only used by
  AdaptiveURISelector. Optional.

``mc_avg_speed``
  The average download speed observed in the previous download in
  bytes per sec. This value is only updated if the download is done in
  multi connection environment and only used by
  AdaptiveURISelector. Optional.

``counter``
  How many times the server is used. Currently this value is only used
  by AdaptiveURISelector.  Optional.

``last_updated``
  Last contact time in GMT with this server, specified in the seconds
  since the Epoch(00:00:00 on January 1, 1970, UTC). Required.

``status``
  ERROR is set when server cannot be reached or out-of-service or
  timeout occurred. Otherwise, OK is set.

Estes campos devem existir em uma linha. A ordem dos campos não importa.
Podem ser colocados pares; eles serão simplesmente ignorados.

Um exemplo abaixo::

  host=localhost, protocol=http, dl_speed=32000, last_updated=1222491640,
  status=OK
  host=localhost, protocol=ftp, dl_speed=0, last_updated=1222491632,
  status=ERROR


.. index:: double: interface; rpc;

INTERFACE RPC
-------------

aria2 provê o serviço JSON-RPC sobre HTTP e XML-RPC sobre HTTP e eles
basicamente possuem a mesma funcionalidade.  aria2 também provê JSON-RPC
sobre WebSocket que utiliza o mesmo formato, do método e assinatura e 
de resposta do formato JSON-RPC sobre HTTP, mas adicionalmente possui 
notificações iniciadas pelo servidor. 
Ver detalhes na seção `JSON-RPC sobre WebSocket`_ .

O caminho requisitado do interface JSON-RPC (sobre HTTP e sobre
WebSocket) é ``/jsonrpc``.  O caminho requisitado do interface  XML-RPC é
``/rpc``.

A URI WebSocket para JSON-RPC sobre WebSocket é ``ws://HOST:PORT/jsonrpc``.

A implementação JSON-RPC é baseada na especificação
``JSON-RPC 2.0 <http://jsonrpc.org/specification>`` e suporta
HTTP POST e GET (JSONP). Usando WebSocket como transporte, é uma extensão
original do aria2.

A interface JSON-RPC não suporta notificação em HTTP, mas o servidor RPC irá
enviar a notificação no WebSocket. Não é suportado número de ponto flutuante
O codificação de página deve ser UTF-8.

Quanto a seguinte documentação do JSON-RPC, entenda estrutura JSON como objeto.

.. index::   single: terminologia

Terminologia
~~~~~~~~~~~~

GID
  GID(or gid) é a chave para gerenciar cada download. Cada download tem um
  único GID. Atualmente GID é armazenado em 64 bits como dado binário no
  aria2. Para acesso RPG, isso é representado em uma string hexadecimal
  de 16 caracteres (exemplo: ``2089b05ecca3d829``). Normalmente, aria2
  gera esse GID para cada download, mas o usuário pode especificar
  o GID manualmente usando a opção :option:`--gid <--gid>`. Quando
  consultando um download por GID, pode ser especificado o prefixo
  do GID como um prefixo único e exclusivo entre outros.

.. index:: double: exemplos; métodos

Métodos
~~~~~~~

São descritos em torno de 35 exemplos, os quais serão numerados
utilizando código fonte com exemplos que usam a linguagem
Python versão 2.7.

.. index::   triple:     exemplo; json-rpc; xml-rpc;

.. function:: aria2.addUri(uris[, options[, position]])

  This method adds new HTTP(S)/FTP/BitTorrent Magnet URI.  *uris* is of
  type array and its element is URI which is of type string.  For
  BitTorrent Magnet URI, *uris* must have only one element and it should
  be BitTorrent Magnet URI.  URIs in *uris* must point to the same file.
  If you mix other URIs which point to another file, aria2 does not
  complain but download may fail.  *options* is of type struct and its
  members are a pair of option name and value. See :ref:`rpc_options` below for
  more details.  If *position* is given as an integer starting from 0,
  the new download is inserted at *position* in the waiting queue. If
  *position* is not given or *position* is larger than the size of the
  queue, it is appended at the end of the queue.  This method returns
  GID of registered download.

  **JSON-RPC EXEMPLO M010**

  The following example adds ``http://example.org/file``::

    >>> import urllib2, json
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.addUri',
    ...                       'params':[['http://example.org/file']]})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> c.read()
    '{"id":"qwer","jsonrpc":"2.0","result":"2089b05ecca3d829"}'

  **XML-RPC EXEMPLO M020**

  The following example adds ``http://example.org/file``::

    >>> import xmlrpclib
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> s.aria2.addUri(['http://example.org/file'])
    '2089b05ecca3d829'

  The following example adds 2 sources and some options::

    >>> s.aria2.addUri(['http://example.org/file', 'http://mirror/file'],
                        dict(dir="/tmp"))
    'd2703803b52216d1'

  The following example adds a download and insert it to the front of
  waiting downloads::

    >>> s.aria2.addUri(['http://example.org/file'], {}, 0)
    'ca3d829cee549a4d'

.. function:: aria2.addTorrent(torrent[, uris[, options[, position]]])

  This method adds BitTorrent download by uploading ".torrent" file.
  If you want to add BitTorrent Magnet URI, use :func:`aria2.addUri`
  method instead.  *torrent* is of type base64 which contains
  Base64-encoded ".torrent" file.  *uris* is of type array and its
  element is URI which is of type string. *uris* is used for
  Web-seeding.  For single file torrents, URI can be a complete URI
  pointing to the resource or if URI ends with /, name in torrent file
  is added. For multi-file torrents, name and path in torrent are
  added to form a URI for each file.  *options* is of type struct and
  its members are a pair of option name and value. See
  :ref:`rpc_options` below for more details.  If *position* is given
  as an integer starting from 0, the new download is inserted at
  *position* in the waiting queue. If *position* is not given or
  *position* is larger than the size of the queue, it is appended at
  the end of the queue.  This method returns GID of registered
  download. If :option:`--rpc-save-upload-metadata` is ``true``, the
  uploaded data is saved as a file named hex string of SHA-1 hash of
  data plus ".torrent" in the directory specified by :option:`--dir
  <-d>` option.  The example of filename is
  ``0a3893293e27ac0490424c06de4d09242215f0a6.torrent``.  If same file
  already exists, it is overwritten.  If the file cannot be saved
  successfully or :option:`--rpc-save-upload-metadata` is ``false``,
  the downloads added by this method are not saved by
  :option:`--save-session`.

  The following examples add local file ``file.torrent``.

  **JSON-RPC EXEMPLO M030**

  ::

    >>> import urllib2, json, base64
    >>> torrent = base64.b64encode(open('file.torrent').read())
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'asdf',
    ...                       'method':'aria2.addTorrent', 'params':[torrent]})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> c.read()
    '{"id":"asdf","jsonrpc":"2.0","result":"2089b05ecca3d829"}'

  **XML-RPC EXEMPLO M040**

  ::

    >>> import xmlrpclib
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> s.aria2.addTorrent(xmlrpclib.Binary(open('file.torrent').read()))
    '2089b05ecca3d829'

.. function:: aria2.addMetalink(metalink[, options[, position]])

  This method adds Metalink download by uploading ".metalink" file.
  *metalink* is of type base64 which contains Base64-encoded
  ".metalink" file.  *options* is of type struct and its members are a
  pair of option name and value. See :ref:`rpc_options` below for more
  details.  If *position* is given as an integer starting from 0, the
  new download is inserted at *position* in the waiting queue. If
  *position* is not given or *position* is larger than the size of the
  queue, it is appended at the end of the queue.  This method returns
  array of GID of registered download.  If
  :option:`--rpc-save-upload-metadata` is ``true``, the uploaded data
  is saved as a file named hex string of SHA-1 hash of data plus
  ".metalink" in the directory specified by :option:`--dir <-d>`
  option.  The example of filename is
  ``0a3893293e27ac0490424c06de4d09242215f0a6.metalink``.  If same file
  already exists, it is overwritten.  If the file cannot be saved
  successfully or :option:`--rpc-save-upload-metadata` is ``false``,
  the downloads added by this method are not saved by
  :option:`--save-session`.

  The following examples add local file file.meta4.

  **JSON-RPC EXEMPLO M050**

  ::

    >>> import urllib2, json, base64
    >>> metalink = base64.b64encode(open('file.meta4').read())
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.addMetalink',
    ...                       'params':[metalink]})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> c.read()
    '{"id":"qwer","jsonrpc":"2.0","result":["2089b05ecca3d829"]}'

  **XML-RPC EXEMPLO M060**

  ::

    >>> import xmlrpclib
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> s.aria2.addMetalink(xmlrpclib.Binary(open('file.meta4').read()))
    ['2089b05ecca3d829']

.. function:: aria2.remove(gid)

  This method removes the download denoted by *gid*. *gid* is of type
  string. If specified download is in progress, it is stopped at
  first. The status of removed download becomes ``removed``.  This method
  returns GID of removed download.

  The following examples remove download GID#2089b05ecca3d829.

  **JSON-RPC EXEMPLO M070**

  ::

    >>> import urllib2, json
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.remove',
    ...                       'params':['2089b05ecca3d829']})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> c.read()
    '{"id":"qwer","jsonrpc":"2.0","result":"2089b05ecca3d829"}'

  **XML-RPC EXEMPLO M080**

  ::

    >>> import xmlrpclib
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> s.aria2.remove('2089b05ecca3d829')
    '2089b05ecca3d829'

.. function:: aria2.forceRemove(gid)

  This method removes the download denoted by *gid*.  This method
  behaves just like :func:`aria2.remove` except that this method removes
  download without any action which takes time such as contacting
  BitTorrent tracker.

.. function:: aria2.pause(gid)

  This method pauses the download denoted by *gid*. *gid* is of type
  string. The status of paused download becomes ``paused``.  If the
  download is active, the download is placed on the first position of
  waiting queue.  As long as the status is ``paused``, the download is not
  started.  To change status to ``waiting``, use :func:`aria2.unpause` method.
  This method returns GID of paused download.

.. function:: aria2.pauseAll()

  This method is equal to calling :func:`aria2.pause` for every active/waiting
  download. This methods returns ``OK`` for success.

.. function:: aria2.forcePause(pid)

  This method pauses the download denoted by *gid*.  This method
  behaves just like :func:`aria2.pause` except that this method pauses
  download without any action which takes time such as contacting
  BitTorrent tracker.

.. function:: aria2.forcePauseAll()

  This method is equal to calling :func:`aria2.forcePause` for every
  active/waiting download. This methods returns ``OK`` for success.

.. function:: aria2.unpause(gid)

  This method changes the status of the download denoted by *gid* from
  ``paused`` to ``waiting``. This makes the download eligible to restart.
  *gid* is of type string.  This method returns GID of unpaused
  download.

.. function:: aria2.unpauseAll()

  This method is equal to calling :func:`aria2.unpause` for every active/waiting
  download. This methods returns ``OK`` for success.

.. function:: aria2.tellStatus(gid[, keys])

  This method returns download progress of the download denoted by
  *gid*. *gid* is of type string. *keys* is array of string. If it is
  specified, the response contains only keys in *keys* array. If *keys*
  is empty or not specified, the response contains all keys.  This is
  useful when you just want specific keys and avoid unnecessary
  transfers. For example, ``aria2.tellStatus("2089b05ecca3d829", ["gid", "status"])``
  returns *gid* and 'status' key.  The response is of type struct and it
  contains following keys. The value type is string.

  ``gid``
    GID of this download.

  ``status``
    ``active`` for currently downloading/seeding entry. ``waiting`` for the
    entry in the queue; download is not started.  ``paused`` for the
    paused entry.  ``error`` for the stopped download because of
    error. ``complete`` for the stopped and completed download. ``removed``
    for the download removed by user.

  ``totalLength``
    Total length of this download in bytes.

  ``completedLength``
    Completed length of this download in bytes.

  ``uploadLength``
    Uploaded length of this download in bytes.

  ``bitfield``
    Hexadecimal representation of the download progress. The highest bit
    corresponds to piece index 0. The set bits indicate the piece is
    available and unset bits indicate the piece is missing. The spare
    bits at the end are set to zero.  When download has not started yet,
    this key will not be included in the response.

  ``downloadSpeed``
    Download speed of this download measured in bytes/sec.

  ``uploadSpeed``
    Upload speed of this download measured in bytes/sec.

  ``infoHash``
    InfoHash. BitTorrent only.

  ``numSeeders``
    The number of seeders the client has connected to. BitTorrent only.

  ``pieceLength``
    Piece length in bytes.

  ``numPieces``
    The number of pieces.

  ``connections``
    The number of peers/servers the client has connected to.

  ``errorCode``
    The last error code occurred in this download. The value is of type
    string. The error codes are defined in `CÓDIGO DE RETORNO ou SAÍDA`_ 
    section. This value is only available for stopped/completed downloads.

  ``followedBy``
    List of GIDs which are generated by the consequence of this
    download. For example, when aria2 downloaded Metalink file, it
    generates downloads described in it(see :option:`--follow-metalink`
    option). This value is useful to track these auto generated
    downloads. If there is no such downloads, this key will not
    be included in the response.

  ``belongsTo``
    GID of a parent download. Some downloads are a part of another
    download.  For example, if a file in Metalink has BitTorrent
    resource, the download of ".torrent" is a part of that file.  If this
    download has no parent, this key will not be included in the
    response.

  ``dir``
    Directory to save files. This key is not available for stopped
    downloads.

  ``files``
    Returns the list of files. The element of list is the same struct
    used in :func:`aria2.getFiles` method.

  ``bittorrent``
    Struct which contains information retrieved from .torrent
    file. BitTorrent only. It contains following keys.

    ``announceList``
      List of lists of announce URI. If ".torrent" file contains announce
      and no announce-list, announce is converted to announce-list
      format.

    ``comment``
      The comment for the torrent. ``comment.utf-8`` is used if available.

    ``creationDate``
      The creation time of the torrent. The value is an integer since
      the Epoch, measured in seconds.

    ``mode``
      File mode of the torrent. The value is either ``single`` or ``multi``.

    ``info``
      Struct which contains data from Info dictionary. It contains
      following keys.

      ``name``
        name in info dictionary. ``name.utf-8`` is used if available.

  **JSON-RPC EXEMPLO M090**

  The following example gets information about download GID#2089b05ecca3d829::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.tellStatus',
    ...                       'params':['2089b05ecca3d829']})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer',
     u'jsonrpc': u'2.0',
     u'result': {u'bitfield': u'0000000000',
                 u'completedLength': u'901120',
                 u'connections': u'1',
                 u'dir': u'/downloads',
                 u'downloadSpeed': u'15158',
                 u'files': [{u'index': u'1',
                             u'length': u'34896138',
                             u'completedLength': u'34896138',
                             u'path': u'/downloads/file',
                             u'selected': u'true',
                             u'uris': [{u'status': u'used',
                                        u'uri': u'http://example.org/file'}]}],
                 u'gid': u'2089b05ecca3d829',
                 u'numPieces': u'34',
                 u'pieceLength': u'1048576',
                 u'status': u'active',
                 u'totalLength': u'34896138',
                 u'uploadLength': u'0',
                 u'uploadSpeed': u'0'}}

  The following example gets information specifying keys you are
  interested in::

    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.tellStatus',
    ...                       'params':['2089b05ecca3d829',
    ...                                 ['gid',
    ...                                  'totalLength',
    ...                                  'completedLength']]})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer',
     u'jsonrpc': u'2.0',
     u'result': {u'completedLength': u'5701632',
                 u'gid': u'2089b05ecca3d829',
                 u'totalLength': u'34896138'}}

  **XML-RPC EXEMPLO M100**

  The following example gets information about download GID#2089b05ecca3d829::

    >>> import xmlrpclib
    >>> from pprint import pprint
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> r = s.aria2.tellStatus('2089b05ecca3d829')
    >>> pprint(r)
    {'bitfield': 'ffff80',
     'completedLength': '34896138',
     'connections': '0',
     'dir': '/downloads',
     'downloadSpeed': '0',
     'errorCode': '0',
     'files': [{'index': '1',
                'length': '34896138',
                'completedLength': '34896138',
                'path': '/downloads/file',
                'selected': 'true',
                'uris': [{'status': 'used',
                          'uri': 'http://example.org/file'}]}],
     'gid': '2089b05ecca3d829',
     'numPieces': '17',
     'pieceLength': '2097152',
     'status': 'complete',
     'totalLength': '34896138',
     'uploadLength': '0',
     'uploadSpeed': '0'}

  The following example gets information specifying keys you are
  interested in::

    >>> r = s.aria2.tellStatus('2089b05ecca3d829', ['gid', 'totalLength', 'completedLength'])
    >>> pprint(r)
    {'completedLength': '34896138', 'gid': '2089b05ecca3d829', 'totalLength': '34896138'}

.. function:: aria2.getUris(gid)

  This method returns URIs used in the download denoted by *gid*.  *gid*
  is of type string. The response is of type array and its element is of
  type struct and it contains following keys. The value type is string.

  ``uri``
    URI

  ``status``
    'used' if the URI is already used. 'waiting' if the URI is waiting
    in the queue.

  **JSON-RPC EXEMPLO M110**
  ::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.getUris',
    ...                       'params':['2089b05ecca3d829']})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer',
     u'jsonrpc': u'2.0',
     u'result': [{u'status': u'used',
                  u'uri': u'http://example.org/file'}]}

  **XML-RPC EXEMPLO M120**
  ::

    >>> import xmlrpclib
    >>> from pprint import pprint
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> r = s.aria2.getUris('2089b05ecca3d829')
    >>> pprint(r)
    [{'status': 'used', 'uri': 'http://example.org/file'}]

.. function:: aria2.getFiles(gid)

  This method returns file list of the download denoted by *gid*. *gid*
  is of type string. The response is of type array and its element is of
  type struct and it contains following keys. The value type is string.

  ``index``
    Index of file. Starting with 1. This is the same order with the
    files in multi-file torrent.

  ``path``
    File path.

  ``length``
    File size in bytes.

  ``completedLength``
    Completed length of this file in bytes.  Please note that it is
    possible that sum of completedLength is less than completedLength in
    :func:`aria2.tellStatus` method.
    This is because completedLength in
    :func:`aria2.getFiles`
    only calculates completed pieces. On the other hand, completedLength
    in
    :func:`aria2.tellStatus` takes into account
    of partially completed piece.

  ``selected``
    ``true`` if this file is selected by :option:`--select-file` option. If
    :option:`--select-file` is not specified or this is single torrent or no
    torrent download, this value is always ``true``. Otherwise ``false``.

  ``uris``
    Returns the list of URI for this file. The element of list is the
    same struct used in :func:`aria2.getUris` method.

  **JSON-RPC EXEMPLO M130**
  ::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.getFiles',
    ...                       'params':['2089b05ecca3d829']})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer',
     u'jsonrpc': u'2.0',
     u'result': [{u'index': u'1',
                  u'length': u'34896138',
                  u'completedLength': u'34896138',
                  u'path': u'/downloads/file',
                  u'selected': u'true',
                  u'uris': [{u'status': u'used',
                             u'uri': u'http://example.org/file'}]}]}

  **XML-RPC EXEMPLO M140**
  ::

    >>> import xmlrpclib
    >>> from pprint import pprint
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> r = s.aria2.getFiles('2089b05ecca3d829')
    >>> pprint(r)
    [{'index': '1',
      'length': '34896138',
      'completedLength': '34896138',
      'path': '/downloads/file',
      'selected': 'true',
      'uris': [{'status': 'used',
                'uri': 'http://example.org/file'}]}]

.. function:: aria2.getPeers(gid)

  This method returns peer list of the download denoted by *gid*. *gid*
  is of type string. This method is for BitTorrent only.  The response
  is of type array and its element is of type struct and it contains
  following keys. The value type is string.

  ``peerId``
    Percent-encoded peer ID.

  ``ip``
    IP address of the peer.

  ``port``
    Port number of the peer.

  ``bitfield``
    Hexadecimal representation of the download progress of the peer. The
    highest bit corresponds to piece index 0. The set bits indicate the
    piece is available and unset bits indicate the piece is missing. The
    spare bits at the end are set to zero.

  ``amChoking``
    ``true`` if this client is choking the peer. Otherwise ``false``.

  ``peerChoking``
    ``true`` if the peer is choking this client. Otherwise ``false``.

  ``downloadSpeed``
    Download speed (byte/sec) that this client obtains from the peer.

  ``uploadSpeed``
    Upload speed(byte/sec) that this client uploads to the peer.

  ``seeder``
    ``true`` is this client is a seeder. Otherwise ``false``.

  **JSON-RPC EXEMPLO M150**
  ::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.getPeers',
    ...                       'params':['2089b05ecca3d829']})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer',
     u'jsonrpc': u'2.0',
     u'result': [{u'amChoking': u'true',
                  u'bitfield': u'ffffffffffffffffffffffffffffffffffffffff',
                  u'downloadSpeed': u'10602',
                  u'ip': u'10.0.0.9',
                  u'peerChoking': u'false',
                  u'peerId': u'aria2%2F1%2E10%2E5%2D%87%2A%EDz%2F%F7%E6',
                  u'port': u'6881',
                  u'seeder': u'true',
                  u'uploadSpeed': u'0'},
                 {u'amChoking': u'false',
                  u'bitfield': u'ffffeff0fffffffbfffffff9fffffcfff7f4ffff',
                  u'downloadSpeed': u'8654',
                  u'ip': u'10.0.0.30',
                  u'peerChoking': u'false',
                  u'peerId': u'bittorrent client758',
                  u'port': u'37842',
                  u'seeder': u'false',
                  u'uploadSpeed': u'6890'}]}

  **XML-RPC EXEMPLO M160**
  ::

    >>> import xmlrpclib
    >>> from pprint import pprint
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> r = s.aria2.getPeers('2089b05ecca3d829')
    >>> pprint(r)
    [{'amChoking': 'true',
      'bitfield': 'ffffffffffffffffffffffffffffffffffffffff',
      'downloadSpeed': '10602',
      'ip': '10.0.0.9',
      'peerChoking': 'false',
      'peerId': 'aria2%2F1%2E10%2E5%2D%87%2A%EDz%2F%F7%E6',
      'port': '6881',
      'seeder': 'true',
      'uploadSpeed': '0'},
     {'amChoking': 'false',
      'bitfield': 'ffffeff0fffffffbfffffff9fffffcfff7f4ffff',
      'downloadSpeed': '8654',
      'ip': '10.0.0.30',
      'peerChoking': 'false',
      'peerId': 'bittorrent client758',
      'port': '37842',
      'seeder': 'false,
      'uploadSpeed': '6890'}]

.. function:: aria2.getServers(gid)

  This method returns currently connected HTTP, HTTPS, FTP servers of the 
  download denoted by *gid*. *gid* is of type string. The response
  is of type array and its element is of type struct and it contains
  following keys. The value type is string.

  ``index``
    Index of file. Starting with 1. This is the same order with the
    files in multi-file torrent.

  ``servers``
    The list of struct which contains following keys.

    ``uri``
      URI originally added.

    ``currentUri``
      This is the URI currently used for downloading. If redirection is
      involved, currentUri and uri may differ.

    ``downloadSpeed``
      Download speed (byte/sec)

  **JSON-RPC EXEMPLO M170**
  ::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.getServers',
    ...                       'params':['2089b05ecca3d829']})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer',
     u'jsonrpc': u'2.0',
     u'result': [{u'index': u'1',
                  u'servers': [{u'currentUri': u'http://example.org/file',
                                u'downloadSpeed': u'10467',
                                u'uri': u'http://example.org/file'}]}]}

  **XML-RPC EXEMPLO M180**
  ::

    >>> import xmlrpclib
    >>> from pprint import pprint
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> r = s.aria2.getServers('2089b05ecca3d829')
    >>> pprint(r)
    [{'index': '1',
      'servers': [{'currentUri': 'http://example.org/dl/file',
                   'downloadSpeed': '20285',
                   'uri': 'http://example.org/file'}]}]

.. function:: aria2.tellActive([keys])

  This method returns the list of active downloads.  The response is of
  type array and its element is the same struct returned by
  :func:`aria2.tellStatus` method. For *keys* parameter, please refer to
  :func:`aria2.tellStatus` method.

.. function:: aria2.tellWaiting(offset, num, [keys])

  This method returns the list of waiting download, including paused
  downloads. *offset* is of type integer and specifies the offset from
  the download waiting at the front. *num* is of type integer and
  specifies the number of downloads to be returned.  For *keys*
  parameter, please refer to :func:`aria2.tellStatus` method.

  If *offset* is a positive integer, this method returns downloads in the
  range of [*offset*, *offset* + *num*).

  *offset* can be a negative integer. *offset* == -1 points last
  download in the waiting queue and *offset* == -2 points the download
  before the last download, and so on. The downloads in the response are
  in reversed order.

  For example, imagine that three downloads "A","B" and "C" are waiting
  in this order. ``aria2.tellWaiting(0, 1)`` returns
  ``["A"]``. ``aria2.tellWaiting(1, 2)`` returns ``["B", "C"]``.
  ``aria2.tellWaiting(-1, 2)`` returns ``["C", "B"]``.

  The response is of type array and its element is the same struct
  returned by :func:`aria2.tellStatus` method.

.. function:: aria2.tellStopped(offset, num, [keys])

  This method returns the list of stopped download.  *offset* is of type
  integer and specifies the offset from the oldest download. *num* is of
  type integer and specifies the number of downloads to be returned.
  For *keys* parameter, please refer to :func:`aria2.tellStatus` method.

  *offset* and *num* have the same semantics as :func:`aria2.tellWaiting`
  method.

  The response is of type array and its element is the same struct
  returned by :func:`aria2.tellStatus` method.

.. function:: aria2.changePosition(gid, pos, how)

  This method changes the position of the download denoted by
  *gid*. *pos* is of type integer. *how* is of type string. If *how* is
  ``POS_SET``, it moves the download to a position relative to the
  beginning of the queue.  If *how* is ``POS_CUR``, it moves the download
  to a position relative to the current position. If *how* is ``POS_END``,
  it moves the download to a position relative to the end of the
  queue. If the destination position is less than 0 or beyond the end of
  the queue, it moves the download to the beginning or the end of the
  queue respectively. The response is of type integer and it is the
  destination position.

  For example, if GID#2089b05ecca3d829 is placed in position 3,
  ``aria2.changePosition('2089b05ecca3d829', -1, 'POS_CUR')`` will
  change its position to 2. Additional
  ``aria2.changePosition('2089b05ecca3d829', 0, 'POS_SET')`` will
  change its position to 0(the beginning of the queue).

  The following examples move the download GID#2089b05ecca3d829 to the
  front of the waiting queue.

  **JSON-RPC EXEMPLO M190**

  ::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.changePosition',
    ...                       'params':['2089b05ecca3d829', 0, 'POS_SET']})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer', u'jsonrpc': u'2.0', u'result': 0}

  **XML-RPC EXEMPLO M200**

  ::

    >>> import xmlrpclib
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> s.aria2.changePosition('2089b05ecca3d829', 0, 'POS_SET')
    0

.. index::   triple:     exemplo; json-rpc; xml-rpc;
             double:     python;  proxy-server;

.. function:: aria2.changeUri(gid, fileIndex, delUris, addUris[, position])

  This method removes URIs in *delUris* from and appends URIs in
  *addUris* to download denoted by *gid*. *delUris* and *addUris* are
  list of string. A download can contain multiple files and URIs are
  attached to each file.  *fileIndex* is used to select which file to
  remove/attach given URIs. *fileIndex* is 1-based. *position* is used
  to specify where URIs are inserted in the existing waiting URI
  list. *position* is 0-based. When *position* is omitted, URIs are
  appended to the back of the list.  This method first execute removal
  and then addition. *position* is the position after URIs are removed,
  not the position when this method is called.  When removing URI, if
  same URIs exist in download, only one of them is removed for each URI
  in *delUris*. In other words, there are three URIs
  ``http://example.org/aria2`` and you want remove them all, you have to
  specify (at least) 3 ``http://example.org/aria2`` in *delUris*.  This
  method returns a list which contains 2 integers. The first integer is
  the number of URIs deleted. The second integer is the number of URIs
  added.

  The following examples add 1 URI ``http://example.org/file`` to the
  file whose index is ``1`` and belongs to the download
  GID#2089b05ecca3d829.

  **JSON-RPC EXEMPLO M210**

  ::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.changeUri',
    ...                       'params':['2089b05ecca3d829', 1, [],
                                        ['http://example.org/file']]})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer', u'jsonrpc': u'2.0', u'result': [0, 1]}

  **XML-RPC EXEMPLO M220**

  ::

    >>> import xmlrpclib
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> s.aria2.changeUri('2089b05ecca3d829', 1, [],
                          ['http://example.org/file'])
    [0, 1]

.. function:: aria2.getOption(gid)

  This method returns options of the download denoted by *gid*.  The
  response is of type struct. Its key is the name of option.  The value
  type is string. Note that this method does not return options which
  have no default value and have not been set by the command-line
  options, configuration files or RPC methods.

  The following examples get options of the download
  GID#2089b05ecca3d829.

  **JSON-RPC EXEMPLO M230**

  ::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.getOption',
    ...                       'params':['2089b05ecca3d829']})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer',
     u'jsonrpc': u'2.0',
     u'result': {u'allow-overwrite': u'false',
                 u'allow-piece-length-change': u'false',
                 u'always-resume': u'true',
                 u'async-dns': u'true',
     ...

  **XML-RPC EXEMPLO M240**

  ::

    >>> import xmlrpclib
    >>> from pprint import pprint
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> r = s.aria2.getOption('2089b05ecca3d829')
    >>> pprint(r)
    {'allow-overwrite': 'false',
     'allow-piece-length-change': 'false',
     'always-resume': 'true',
     'async-dns': 'true',
     ....

.. function:: aria2.changeOption(gid, options)

  This method changes options of the download denoted by *gid*
  dynamically.  *gid* is of type string.  *options* is of type struct.
  The following options are available for active downloads:

  * :option:`bt-max-peers <--bt-max-peers>`
  * :option:`bt-request-peer-speed-limit <--bt-request-peer-speed-limit>`
  * :option:`bt-remove-unselected-file <--bt-remove-unselected-file>`
  * :option:`force-save <--force-save>`
  * :option:`max-download-limit <--max-download-limit>`
  * :option:`max-upload-limit <-u>`

  For waiting or paused downloads, in addition to the above options,
  options listed in `Arquivo de Entrada`_ subsection are available,
  except for following options:
  :option:`dry-run <--dry-run>`,
  :option:`metalink-base-uri <--metalink-base-uri>`,
  :option:`parameterized-uri <-P>`,
  :option:`pause <--pause>`,
  :option:`piece-length <--piece-length>` and
  :option:`rpc-save-upload-metadata <--rpc-save-upload-metadata>` option.
  This method returns ``OK`` for success.

  The following examples set :option:`max-download-limit
  <--max-download-limit>` option to ``20K`` for the download
  GID#2089b05ecca3d829.

  **JSON-RPC EXEMPLO M250**

  ::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.changeOption',
    ...                       'params':['2089b05ecca3d829',
    ...                                 {'max-download-limit':'10K'}]})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer', u'jsonrpc': u'2.0', u'result': u'OK'}

  **XML-RPC EXEMPLO M260**

  ::

    >>> import xmlrpclib
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> s.aria2.changeOption('2089b05ecca3d829', {'max-download-limit':'20K'})
    'OK'

.. function:: aria2.getGlobalOption()

  This method returns global options.  The response is of type
  struct. Its key is the name of option.  The value type is string.
  Note that this method does not return options which have no default
  value and have not been set by the command-line options, configuration
  files or RPC methods. Because global options are used as a template
  for the options of newly added download, the response contains keys
  returned by :func:`aria2.getOption` method.

.. function:: aria2.changeGlobalOption(options)

  This method changes global options dynamically.  *options* is of type
  struct.
  The following options are available:

  * :option:`download-result <--download-result>`
  * :option:`log <-l>`
  * :option:`log-level <--log-level>`
  * :option:`max-concurrent-downloads <-j>`
  * :option:`max-download-result <--max-download-result>`
  * :option:`max-overall-download-limit <--max-overall-download-limit>`
  * :option:`max-overall-upload-limit <--max-overall-upload-limit>`
  * :option:`save-cookies <--save-cookies>`
  * :option:`save-session <--save-session>`
  * :option:`server-stat-of <--server-stat-of>`

  In addition to them, options listed in `Arquivo de Entrada`_ subsection
  are available, except for following options:
  :option:`checksum <--checksum>`,
  :option:`index-out <-O>`,
  :option:`out <-o>`,
  :option:`pause <--pause>` and
  :option:`select-file <--select-file>`.

  Using :option:`log <-l>` option, you can dynamically start logging or
  change log file. To stop logging, give empty string("") as a parameter
  value. Note that log file is always opened in append mode. This method
  returns ``OK`` for success.

.. function:: aria2.getGlobalStat()

  This method returns global statistics such as overall download and
  upload speed. The response is of type struct and contains following
  keys. The value type is string.

  ``downloadSpeed``
    Overall download speed (byte/sec).

  ``uploadSpeed``
    Overall upload speed(byte/sec).

  ``numActive``
    The number of active downloads.

  ``numWaiting``
    The number of waiting downloads.

  ``numStopped``
    The number of stopped downloads.

  **JSON-RPC EXEMPLO M270**
  ::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.getGlobalStat'})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer',
     u'jsonrpc': u'2.0',
     u'result': {u'downloadSpeed': u'21846',
                 u'numActive': u'2',
                 u'numStopped': u'0',
                 u'numWaiting': u'0',
                 u'uploadSpeed': u'0'}}

  **XML-RPC EXEMPLO M280**
  ::

    >>> import xmlrpclib
    >>> from pprint import pprint
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> r = s.aria2.getGlobalStat()
    >>> pprint(r)
    {'downloadSpeed': '23136',
     'numActive': '2',
     'numStopped': '0',
     'numWaiting': '0',
     'uploadSpeed': '0'}

.. function:: aria2.purgeDownloadResult()

  This method purges completed/error/removed downloads to free memory.
  This method returns ``OK``.

.. function:: aria2.removeDownloadResult(gid)

  This method removes completed/error/removed download denoted by *gid*
  from memory. This method returns ``OK`` for success.

  The following examples remove the download result of the download
  GID#2089b05ecca3d829.

  **JSON-RPC EXEMPLO M290**

  ::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.removeDownloadResult',
    ...                       'params':['2089b05ecca3d829']})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer', u'jsonrpc': u'2.0', u'result': u'OK'}

  **XML-RPC EXEMPLO M300**

  ::

    >>> import xmlrpclib
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> s.aria2.removeDownloadResult('2089b05ecca3d829')
    'OK'

.. function:: aria2.getVersion()

  This method returns version of the program and the list of enabled
  features. The response is of type struct and contains following keys.

  ``version``
    Version number of the program in string.

  ``enabledFeatures``
    List of enabled features. Each feature name is of type string.

  **JSON-RPC EXEMPLO M310**
  ::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.getVersion'})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer',
     u'jsonrpc': u'2.0',
     u'result': {u'enabledFeatures': [u'Async DNS',
                                      u'BitTorrent',
                                      u'Firefox3 Cookie',
                                      u'GZip',
                                      u'HTTPS',
                                      u'Message Digest',
                                      u'Metalink',
                                      u'XML-RPC'],
                 u'version': u'1.11.0'}}

  **XML-RPC EXEMPLO M320**
  ::

    >>> import xmlrpclib
    >>> from pprint import pprint
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> r = s.aria2.getVersion()
    >>> pprint(r)
    {'enabledFeatures': ['Async DNS',
                         'BitTorrent',
                         'Firefox3 Cookie',
                         'GZip',
                         'HTTPS',
                         'Message Digest',
                         'Metalink',
                         'XML-RPC'],
     'version': '1.11.0'}

.. function:: aria2.getSessionInfo()

  This method returns session information.
  The response is of type struct and contains following key.

  ``sessionId``
    Session ID, which is generated each time when aria2 is invoked.

  **JSON-RPC EXEMPLO M330**
  ::

    >>> import urllib2, json
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'aria2.getSessionInfo'})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer',
     u'jsonrpc': u'2.0',
     u'result': {u'sessionId': u'cd6a3bc6a1de28eb5bfa181e5f6b916d44af31a9'}}

  **XML-RPC EXEMPLO M340**
  ::

    >>> import xmlrpclib
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> s.aria2.getSessionInfo()
    {'sessionId': 'cd6a3bc6a1de28eb5bfa181e5f6b916d44af31a9'}

.. function:: aria2.shutdown()

  This method shutdowns aria2.  This method returns ``OK``.

.. function:: aria2.forceShutdown()

  This method shutdowns :func:`aria2. This method behaves like  aria2.shutdown`
  except that any actions which takes time such as contacting BitTorrent
  tracker are skipped. This method returns ``OK``.

.. function:: system.multicall(methods)

  This methods encapsulates multiple method calls in a single request.
  *methods* is of type array and its element is struct.  The struct
  contains two keys: ``methodName`` and ``params``.  ``methodName`` is the
  method name to call and ``params`` is array containing parameters to the
  method.  This method returns array of responses.  The element of array
  will either be a one-item array containing the return value of each
  method call or struct of fault element if an encapsulated method call
  fails.

  In the following examples, we add 2 downloads. First one is
  ``http://example.org/file`` and second one is ``file.torrent``.

  **JSON-RPC EXEMPLO M350**
  ::

    >>> import urllib2, json, base64
    >>> from pprint import pprint
    >>> jsonreq = json.dumps({'jsonrpc':'2.0', 'id':'qwer',
    ...                       'method':'system.multicall',
    ...                       'params':[[{'methodName':'aria2.addUri',
    ...                                   'params':[['http://example.org']]},
    ...                                  {'methodName':'aria2.addTorrent',
    ...                                   'params':[base64.b64encode(open('file.torrent').read())]}]]})
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    {u'id': u'qwer', u'jsonrpc': u'2.0', u'result': [[u'2089b05ecca3d829'], [u'd2703803b52216d1']]}

  JSON-RPC also supports Batch request described in JSON-RPC 2.0 Specification::

    >>> jsonreq = json.dumps([{'jsonrpc':'2.0', 'id':'qwer',
    ...                        'method':'aria2.addUri',
    ...                        'params':[['http://example.org']]},
    ...                       {'jsonrpc':'2.0', 'id':'asdf',
    ...                        'method':'aria2.addTorrent',
    ...                        'params':[base64.b64encode(open('file.torrent').read())]}])
    >>> c = urllib2.urlopen('http://localhost:6800/jsonrpc', jsonreq)
    >>> pprint(json.loads(c.read()))
    [{u'id': u'qwer', u'jsonrpc': u'2.0', u'result': u'2089b05ecca3d829'},
     {u'id': u'asdf', u'jsonrpc': u'2.0', u'result': u'd2703803b52216d1'}]

  **XML-RPC EXEMPLO M360**
  ::

    >>> import xmlrpclib
    >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
    >>> mc = xmlrpclib.MultiCall(s)
    >>> mc.aria2.addUri(['http://example.org/file'])
    >>> mc.aria2.addTorrent(xmlrpclib.Binary(open('file.torrent').read()))
    >>> r = mc()
    >>> tuple(r)
    ('2089b05ecca3d829', 'd2703803b52216d1')

Tratamento de Erros
~~~~~~~~~~~~~~~~~~~

Usando JSON-RPC, aria2 retorna objeto JSON que contém código de erro
no código e a mensagem de erro na mensagem.

Usando XML-RPC, aria2 retorna código de falha (faultCode=1) e a mensagem
de erro em (faultString).

.. _rpc_options:

Opções
~~~~~~

Same options for :option:`--input-file <-i>` list are available. 
Ver subseção `Arquivo de Entrada`_ para lista completa de opções.

In the option struct, name element is option name(without preceding
``--``) and value element is argument as string.

EXEMPLO JSON-RPC M370
^^^^^^^^^^^^^^^^^^^^^
::

  {'split':'1', 'http-proxy':'http://proxy/'}


EXEMPLO XML-RPC M380
^^^^^^^^^^^^^^^^^^^^
.. code-block:: xml

  <struct>
    <member>
      <name>split</name>
      <value><string>1</string></value>
    </member>
    <member>
      <name>http-proxy</name>
      <value><string>http://proxy/</string></value>
    </member>
  </struct>


:option:`header <--header>` and :option:`index-out <-O>`
option are allowed multiple times in
command-line. Since name should be unique in struct(many XML-RPC
library implementation uses hash or dict for struct), single string is
not enough. To overcome this situation, they can take array as value
as well as string.

EXEMPLO JSON-RPC M390
^^^^^^^^^^^^^^^^^^^^^
::

  {'header':['Accept-Language: ja', 'Accept-Charset: utf-8']}


EXEMPLO XML-RPC M400
^^^^^^^^^^^^^^^^^^^^
.. code-block:: xml

  <struct>
    <member>
      <name>header</name>
      <value>
        <array>
          <data>
            <value><string>Accept-Language: ja</string></value>
            <value><string>Accept-Charset: utf-8</string></value>
          </data>
        </array>
      </value>
    </member>
  </struct>


Following example adds a download with 2 options: dir and header.
header option has 2 values, so it uses a list::

  >>> import xmlrpclib
  >>> s = xmlrpclib.ServerProxy('http://localhost:6800/rpc')
  >>> opts = dict(dir='/tmp',
  ...             header=['Accept-Language: ja',
  ...                     'Accept-Charset: utf-8'])
  >>> s.aria2.addUri(['http://example.org/file'], opts)
  '1'


JSON-RPC usando HTTP GET
~~~~~~~~~~~~~~~~~~~~~~~~

The JSON-RPC interface also supports request via HTTP GET.
The encoding scheme in GET parameters is based on JSON-RPC over HTTP 
Specification [2008-1-15(RC1)]. The encoding of GET parameters are follows::

  /jsonrpc?method=METHOD_NAME&id=ID&params=BASE64_ENCODED_PARAMS


The ``method`` and ``id`` are always treated as JSON string and their
encoding must be UTF-8.

For example, The encoded string of
``aria2.tellStatus('2089b05ecca3d829')`` with ``id='foo'`` looks like
this::

  /jsonrpc?method=aria2.tellStatus&id=foo&params=WyIyMDg5YjA1ZWNjYTNkODI5Il0%3D


The ``params`` parameter is Base64-encoded JSON array which usually
appears in ``params`` attribute in JSON-RPC request object.  In the
above example, the params is ``["2089b05ecca3d829"]``, therefore::

  ["2089b05ecca3d829"] --(Base64)--> WyIyMDg5YjA1ZWNjYTNkODI5Il0=
               --(Percent Encode)--> WyIyMDg5YjA1ZWNjYTNkODI5Il0%3D


The JSON-RPC interface supports JSONP. You can specify the callback
function in ``jsoncallback`` parameter::

  /jsonrpc?method=aria2.tellStatus&id=foo&params=WyIyMDg5YjA1ZWNjYTNkODI5Il0%3D&jsoncallback=cb

For Batch request, ``method`` and ``id`` parameter must not be specified.
Whole request must be specified in ``params`` parameter. For example,
Batch request::

  [{'jsonrpc':'2.0', 'id':'qwer', 'method':'aria2.getVersion'},
   {'jsonrpc':'2.0', 'id':'asdf', 'method':'aria2.tellActive'}]


will be encoded like this::

  /jsonrpc?params=W3sianNvbnJwYyI6ICIyLjAiLCAiaWQiOiAicXdlciIsICJtZXRob2QiOiAiYXJpYTIuZ2V0VmVyc2lvbiJ9LCB7Impzb25ycGMiOiAiMi4wIiwgImlkIjogImFzZGYiLCAibWV0aG9kIjogImFyaWEyLnRlbGxBY3RpdmUifV0%3D

.. index:: double: json; websocket;

JSON-RPC sobre WebSocket
~~~~~~~~~~~~~~~~~~~~~~~~

JSON-RPC sobre WebSocket utiliza o mesmo método de assinatura e resposta
do formato JSON-RPC sobre HTTP. O WebSocket suportado é da versão 13
o qual é detalhado na :rfc:`6455`.

Para enviar uma requisição RPC para um servidor RPC, enviar strings serializadas
JSON num frame Text. A resposta do servidor RPC será entregue também em um frame
Text.

O servidor RPC ira enviar a notificação ao cliente. A notificação é 
unidirecional, portanto o cliente que receber a notificação não pode
responde-la. Esse método de assinatura de notificação é muito usual mas
não provê uma identificação de chave. Os valores associados pelos parâmetros
chave são os dados que a notificação porta. O formato desses valores
variam dependendo do método de notificação. Os seguintes métodos de notificação
são definidos: 


.. function:: aria2.onDownloadStart(event)

  
  Essa notificação será enviada quando e se o download for iniciado.
  *event* é o tipo de estrutura e pode conter as seguintes chaves:
  O formato do valor é string.
  
  ``gid``
    GID do download.
  

.. function:: aria2.onDownloadPause(event)

  
  Esta notificação será enviada se o download for pausado.  *event*
  tem a mesma estrutura do argumento *event* do método da função
  :func:`aria2.onDownloadStart`.
  

.. function:: aria2.onDownloadStop(event)

  
  Essa notificação será enviada se o download for interrompido pelo usuário.
  *event* tem a mesma estrutura do argumento *event* do método da função
  :func:`aria2.onDownloadStart`.
  

.. function:: aria2.onDownloadComplete(event)

  
  Esta notificação será enviada quando o download for completado.  Para 
  downloads BitTorrent, esta notificação será enviada quando for completado e
  o (seed) terminar. O *event* tem a mesma estrutura do *event* do método da
  função :func:`aria2.onDownloadStart`.
  

.. function:: aria2.onDownloadError(event)

  
  Esta notificação será enviada se o download for interrompido por causa de
  um erro.
  O *event* tem a mesma estrutura do *event* do método da função
  :func:`aria2.onDownloadStart`.
  

.. function:: aria2.onBtDownloadComplete(event)

  
  Esta notificação será enviada se o download for completado para o
  BitTorrent (mas o seeding pode não ter sido concluído).  O *event* tem a 
  mesma estrutura do *event* do método da função :func:`aria2.onDownloadStart`.
  
Exemplo Cliente XML-RPC Ruby
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

O seguinte script Ruby script adiciona ``http://localhost/aria2.tar.bz2`` em
aria2c no servidor em localhost com a opção :option:`--dir=/downloads <-d>` e
imprime a resposta do processamento:

.. code-block:: ruby

  #!/usr/bin/env ruby
  
  require 'xmlrpc/client'
  require 'pp'
  
  client=XMLRPC::Client.new2("http://localhost:6800/rpc")
  
  options={ "dir" => "/downloads" }
  result=client.call("aria2.addUri", [ "http://localhost/aria2.tar.bz2" ], options)
  
  pp result


Se você usa Python, pode usar xmlrpclib (em Python3.x, use xmlrpc.client) para
interagir com aria2::

  import xmlrpclib
  from pprint import pprint
  
  s = xmlrpclib.ServerProxy("http://localhost:6800/rpc")
  r = s.aria2.addUri(["http://localhost/aria2.tar.bz2"], {"dir":"/downloads"})
  pprint(r)

.. index:: double: mensagens; console;

DIVERSOS
--------

Mensagens na Console
~~~~~~~~~~~~~~~~~~~~

Enquanto executa o download de arquivos, aria2 imprime mensagens na console
para mostrar o progresso dos downloads. Um exemplo abaixo::

    [#1 SIZE:400.0KiB/33.2MiB(1%) CN:1 SPD:115.7KiBs ETA:4m51s]

Entenda o que estes números e strings significam.

``#N``
  N significa GID, o qual é um ID único para cada download.

``SIZE``
  Tamanho Total e Tamanho em bytes. Se a :option:`--select-file` é usada,
  será exibida a somatória do tamanho do arquivo.

``SEEDING``
  Taxa compartilhamento ratio. O cliente está funcionando. Após término do 
  download do BitTorrent, ``SIZE`` será substituído por ``SEEDING``.

``CN``
  Número de conexões que o cliente estabeleceu.

``SEED``
  O número de seeders ao qual o cliente está conectado.

``SPD``
  Velocidade do download.

``UP``
  Velocidade e número de bytes transmitidos upload.

``ETA``
  Tempo previsto para conclusão.

``TOTAL SPD``
  A soma das velocidades de download para todos downloads paralelos.

Quando aria2 está alocando o espaço para arquivo ou validando o checksum, 
adicionalmente exibirá o progresso:

FileAlloc
  GID, tamanho alocado e total em bytes.

Checksum
  GID, tamanho validado e total em bytes.

EXEMPLOS DOWNLOAD HTTP / FTP
----------------------------

Download Segmentado HTTP/FTP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Download de arquivo
^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c "http://servidor/arquivo.zip"


.. note::

  Para parar o download, pressione :kbd:`Ctrl-C`. Posteriormente pode ser 
  retomado o mesmo download no mesmo diretório. Podem ser modificadas as URIs
  pois elas apontam para o mesmo arquivo.

Download de arquivo de 2 servidores HTTP diferentes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c "http://servidor/arquivo.zip" "http://espelhobrasil/arquivo.zip"


Download de arquivo do mesmo servidor HTTP usando 2 conexões
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c -x2 -k1M "http://servidorbrasil/arquivo.zip"


Download de arquivo de servidor HTTP e FTP
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c "http://svrbrasil/arquivo.zip" "ftp://servebr/arquivo.zip"


Download arquivos especificados num arquivo txt concomitantemente
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c -i arquivo.txt -j2

.. note::

  -j especifica número de downloads paralelos.

Usando proxy
^^^^^^^^^^^^
Para HTTP:

.. code-block:: console

  $ aria2c --http-proxy="http://svrproxy:8080" "http://servidor/arquivo"


.. code-block:: console

  $ aria2c --http-proxy="http://svrproxy:8080" 
  --no-proxy="localhost,127.0.0.1,192.168.0.0/16" "http://servidor/arquivo"

  a máscara de rede /16 quer dizer que para toda a rede 192.168 também não
  será usado o servidor proxy

Para FTP:

.. code-block:: console

  $ aria2c --ftp-proxy="http://svrproxy:8080" "ftp://servidor/arquivo"


.. note::
  
  Ver :option:`--http-proxy`, :option:`--https-proxy`, :option:`--ftp-proxy`,
  :option:`--all-proxy` e :option:`--no-proxy` para detalhes.  Proxy pode ser
  especificado nas variáveis de ambiente. Ver seção `VARIÁVEIS DE AMBIENTE`_ .

Proxy com autenticação / autorização
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --http-proxy="http://usuário:senha@svrproxy:8080" 
  "http://servidor/arquivo"


.. code-block:: console

  $ aria2c --http-proxy="http://svrproxy:8080" 
  --http-proxy-user="usuário" --http-proxy-passwd="senha" 
  "http://servidor/arquivo"


Download Metalink
~~~~~~~~~~~~~~~~~
Download arquivos com Metalink remoto
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --follow-metalink=mem "http://servidor/arquivo.metalink"


Download arquivos com Metalink local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c -p --lowest-speed-limit=4000 arquivo.metalink

.. note::

  Para parar o download, pressione :kbd:`Ctrl-C`.
  A transferência pode ser retomada executando aria2c com o mesmo argumento 
  no mesmo diretório

Download diversos arquivos Metalink local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c -j2 arquivo1.metalink arquivo2.metalink


Download só arquivos selecionados usando index
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --select-file=1-4,8 arquivo.metalink

.. note::

  O index é exibido na console usando opção -S.

Download um arquivo usando Metalink local com preferência do usuário
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --metalink-location=pt,us --metalink-version=1.1 --metalink-language=pt-BR arquivo.metalink


Download BitTorrent
~~~~~~~~~~~~~~~~~~~
Download arquivos de BitTorrent remotos
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --follow-torrent=mem "http://servidortorrent/arquivo.torrent"


Download usando arquivo torrent local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --max-upload-limit=40K arquivo.torrent

.. note::

  --max-upload-limit especifica taxa máxima de transmissão (upload).

.. note::

  Para parar o download, pressione :kbd:`Ctrl-C`. A transferência pode ser retomada
  ao executar aria2c com os mesmos argumentos no mesmo diretório.

Download usando URI BitTorrent Magnet
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c "magnet:?xt=urn:btih:248D0A1CD08284299DE78D5C1ED359BB46717D8C&dn=aria2"


.. note::

  Lembre-se inserir delimitadores na URI BitTorrent Magnet, pois ela inclui ``&``
  que tem significado de parâmetro. Utilizar apóstrofo(``'``) ou aspas(``"``).

Download 2 torrents
^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c -j2 arquivo1.torrent arquivo2.torrent


Download um arquivo usando torrent e servidor HTTP/FTP
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c -T arqfile.torrent "http://serv1/arqfile" "ftp://svr2/arqfile"

.. note::

  Download de arquivos múltiplos torrent com HTTP e FTP não é suportado.

Download arquivos selecionados usando index(chamado "download seletivo")
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --select-file=1-4,8 arquivo.torrent

.. note::

  O index é exibido na console usando-se a opção -S.

Especificar arquivo saída
^^^^^^^^^^^^^^^^^^^^^^^^^

Para especificar arquivo de saída em Downloads de BitTorrent, faz-se necessário
conhecer o index do arquivo no torrent usando a opção :option:`--show-files <-S>`.
Por exemplo, a saída exibirá algo como::

  idx|path/length
  ===+======================
    1|dist/base-2.6.18.iso
     |99.9MiB
  ---+----------------------
    2|dist/driver-2.6.18.iso
     |169.0MiB
  ---+----------------------


Para salvar 'dist/base-2.6.18.iso' em '/tmp/meudir/base.iso' e
'dist/driver-2.6.18.iso' em '/tmp/dir/driver.iso', use o seguinte comando:

.. code-block:: console

  $ aria2c --dir=/tmp --index-out=1=meudir/base.iso --index-out=2=dir/driver.iso arquivo.torrent


Modificar porta escuta para peer de entrada
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --listen-port=7000-7001,8000 arquivo.torrent

.. note::

  Já que aria2 não configura o firewall ou porta de roteamento para portas 
  de encaminhamento, isto deve ser explicitado manualmente por você.

Especificar condição para para o programa torrent após término do download
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --seed-time=120 --seed-ratio=1.0 arquivo.torrent


.. note::

  No exemplo acima, o programa termina após transcorrer 120 minutos após 
  término do download ou taxa chegar a 1.0.

Controlar velocidade upload Torrent
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --max-upload-limit=100K arquivo.torrent


Habilitar IPv4 DHT
^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --enable-dht --dht-listen-port=6881 arquivo.torrent

.. note::

  DHT utiliza a porta udp, como o aria2 não configura firewall nem porta de roteamento
  ou forwarding, por favor executar estas configurações manualmente.

Habilitar IPv6 DHT
^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --enable-dht6 --dht-listen-port=6881 --dht-listen-addr6=YOUR_GLOBAL_UNICAST_IPV6_ADDR --enable-async-dns6

.. note::

  Se aria2c não foi compilado com c-ares, a opção :option:`--enable-async-dns6` 
  não é necessária. aria2 compartilha a mesma porta ente IPv4 e IPv6 DHT.

Adicionar e remover rastreador URI
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Remover todos os rastreadores (tracker) das URIs descritas no arquivo.torrent
utilize ``http://tracker1/announce`` e ``http://tracker2/announce``

.. code-block:: console

  $ aria2c --bt-exclude-tracker="*" --bt-tracker="http://tracker1/announce,http://tracker2/announce" file.torrent


Funcionalidades avançadas HTTP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Carregar cookies
^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --load-cookies=cookies.txt "http://servidor/arquivo.zip"

.. note::

  Podem ser utilizados sem nenhuma modificação coookies dos navegadores:
  Firefox / Mozilla / Chromium.

Continuar download iniciado por navegadores ou outros programas
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  Quando desejar continuar ou retomar um download cujo processamento foi
  interrompido, seja em navegador ou qualquer outro programa utilize
  o aria2c para retomar este download do ponto onde parou.
  
.. code-block:: console

  $ aria2c -c -s2 "http://servidor/arquivodedownloadparcial.zip"


Autenticação certificado para Cliente SSL/TLS
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --certificate=/path/para/mycert.pem --private-key=/path/para/mykey.pem https://servidor/arquivo

.. note::

  O arquivo especificado na opção :option:`--private-key` não pode estar
  criptografado.  O comportamento do processo fica indefinido quando o 
  arquivo estiver criptografado.

Verificar peer em SSL/TLS usando certificados CA
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --ca-certificate=/path/para/ca-certificates.crt 
  --check-certificate https://servidor/arquivo


Funcionalidades avançadas adicionais
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Controlar velocidade de download
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Quando for necessário o controle da utilização da banda disponível, pode ser
utilizado a opção abaixo. Atenção o sufixo K ou M deve ser em letra maiúscula. 

.. code-block:: console

  $ aria2c --max-download-limit=100K arquivo.metalink


Reparar um download danificado
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c -V arquivo.metalink

.. note::
  
  Reparar downloads danificados pode ser mais eficiente usando
  BitTorrent ou Metalink com a opção verificação (checksums).

Desconectar conexão se a velocidade download for menor que um valor
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --lowest-speed-limit=10K file.metalink


Suporte a URI parametrizada
^^^^^^^^^^^^^^^^^^^^^^^^^^^
A URI pode ser especificada como partes de um conjunto:

.. code-block:: console

  $ aria2c -P "http://{svr1,svr2,svr3}/arquivo.iso"


Também podem ser especificados sequencias de intervalos:

.. code-block:: console

  $ aria2c -Z -P "http://servidor/imagem[000-100].png"


.. note::

  -Z opção requerida para que todas URIs não apontem para o mesmo arquivo,
  como declarado no código acima.

Especificar incrementos para contador:

.. code-block:: console

  $ aria2c -Z -P "http://servidor/imagem[A-Z:2].png"


Verificar validação checksum
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c --checksum=sha-1=0192ba11326fe2298c8cb4de616f4d4140213837 
  http://dobrasil.org/arquivo


Download Paralelo de uma quantidade arbitrária de URI, Metalink ou Torrent
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: console

  $ aria2c -j3 -Z "http://servidor/arquivo1" arquivo2.torrent arq3.metalink


BitTorrent Criptografado
^^^^^^^^^^^^^^^^^^^^^^^^
Criptografar todo conjunto usando ARC4:

.. code-block:: console

  $ aria2c --bt-min-crypto-level=arc4 --bt-require-crypto=true arquivo.torrent


Ver Também
----------

Site do Projeto aria2: https://aria2.github.io/

Site do Projeto Metalink: http://www.metalinker.org/

Descrição do Formato Download Metalink: :rfc:`5854`

COPYRIGHT
---------

Copyright (C) 2006, 2014 Tatsuhiro Tsujikawa
Tradução para Português do Brasil 2013, Gilberto dos Santos Alves
utilizando editor kate e gedit no Debian squeeze 6.0.6 novembro de 2012
revisado em março de 2013 usando editor kate e gedit no ubuntu 12.04 LTS

Esse programa é software livre; pode ser redistribuido e/ou modificado
sob os termos da Licença GNU General Public License como publicada por
Free Software Foundation www.fsf.org; versão 2 da Licença, ou qualquer
versão mais recente, qualquer que seja sua escolha.

Este programa é distribuído na intenção de ser útil, mas SEM NENHUMA GARANTIA;
sem qualquer garantia implícita de ser COMERCIALIZÁVEL ou para PROPÓSITO
ESPECÍFICO. Consulte a Linceça GNU Genérica para mais detalhes.

Você precisa receber uma cópia da Licença Pública GNU Genérica junto com
este programa; caso não tenha, escrevá para Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Adicionalmente, como uma exceção especial, os detentores do direito autoral
autorizam a permissão para compilar programas que possuem ligação com
parte do código com a biblioteca OpenSSL sob certas condições como descritas
em cada arquivo fonte e autorizam a distribuição das das combinações das 
ligações incluindo ambas.
Devem ser obedecidos todos os detalhes da Licença Pública GNU Genérica
em relação a OpenSSL.  Caso haja modificação nos arquivos com esta exceção
deverá ser extendida esta exceção para as versões modificadas dos arquivos, mas
isto não é obrigatório.  Se não houver esta intenção exclua esta declaração
de exceção da sua versão.  Caso sejam excluídas as declarações de todos
os arquivos fontes, exclua também esta declaração daqui.

Anotação sobre divergência entre Manual e o aria2:

Esta página de manual pode não necessariamente conter a última informação.
Caso haja discrepância entre alguma informação do manual e o aria2, por
favor refira-se a versão em inglês resultante do comando man aria2c
