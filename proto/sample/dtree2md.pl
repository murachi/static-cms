use strict;
use utf8;

my %ext2fence = (
  cpp => sub { $_[0]->{'c++'} = 1; 'c++' },
  h => sub { exists $_[0]->{'c++'} ? 'c++' :'c' },
  csproj => sub { 'xml' },
  cs => sub { 'c#' }
  stetic => sub { 'xml' },
  pl => sub { 'perl' },
  py => sub { 'python' },
  rs => sub { 'rust' },
  c => sub { 'c' },
  html => sub { 'html' },
  config => sub { 'xml' },
  xsd => sub { 'xml' },
  resx => sub { 'xml' },
  settings => sub { 'xml' },
  xaml => sub { 'xml' },
  md => sub { 'markdown' },
  scss => sub { 'css' },
  sass => sub { 'css' },
  ini => sub { 'ini' },
  svg => sub { 'xml' },
  sh => sub { 'sh' },
  ts => sub { 'typescript' },
  js => sub { 'javascript' },
  json => sub { 'json' },
  yml => sub { 'yaml' },
  vue => sub { 'html' },
  php => sub { 'php' },
  toml => sub { 'toml' },
);

my @pat2fence = (
  { pat => qr/\.ini\.sample$/, fence => 'ini' },
),

my %fn2fence = (
  Pipfile => sub { 'toml' },
  'Pipfile.lock' => sub { 'json' },
  'Cargo.lock' => sub { 'toml' },
  'composer.lock' => sub { 'json' },
  '.babelrc' => sub { 'json' },
  '.editorconfig' => sub { 'toml' },
  '.prettierrc' => sub { 'json' },
  'Vagrantfile' => sub { 'ruby' },
);

my %ng_ext = qw(sdf 1 keepme 1 png 1 webp 1 ico 1);
