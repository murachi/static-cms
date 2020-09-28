use strict;
use utf8;
use List::Util;

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

my @ignore_pattern;

sub walk {
  my ($cd, $depth) = @_;
  opendir my $dh, $cd or die "ディレクトリ $cd を開けません";
  print "#" x ($depth + 1), " `$cd`\n\n";
  my @items = grep { !/^\.+$/ } readdir $dh;
  ITEMS_LOOP:
  for my $file (@items) {
    my $item = "$cd/$file";
    # .ignore 適用対象を除外
    for my ($base, $patterns) (map { @$_{qw(base patterns)} } @ignore_pattern) {
      my $is_skip = 0;
      for my $pat (@$patterns) {
        my $hit_flag = !$pat =~ /^\~/;
        $pat = $hit_flag ? $pat : substr $pat, 1;
        $pat =~ s/\*/.*?/g;
        $pat =~ s!/$!!;
        if ($pat =~ m!^/!) {
          $is_skip = $item =~ m!^\Q$base\E$pat(/.*|$)! ? $hit_flag : $is_skip;
        }
        else {
          $is_skip = $item =~ m!^\Q$base\E/(.*?/)?$pat(/.*|$)! ? $hit_flag : $is_skip;
        }
      }
      next ITEMS_LOOP if $is_skip;
    }
    walk($item, $depth + 1), next if -d $item;

    my ($ext) = $file =~ /\.(.*?)$/ || '';
    next if exists $ng_ext{$ext};

    open my $fin, "<$item" or die "ファイル $item を開けません";
    print "- $file\n  ```";
    my $fence = exists $fn2fence{$file} ? $fn2fence{$file}->() : '';
  }
}

print STDERR ("収集するソースツリーがあるディレクトリを指定してください。"), exit 1
  unless @ARGV;
my $root = $ARGV[0];
print STDERR ("存在するディレクトリを指定してください。"), exit 1
  unless -d $root;
