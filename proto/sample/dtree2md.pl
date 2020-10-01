use strict;
use List::Util qw/any/;

my %ext2fence = (
  cpp => sub { $_[0]->{'c++'} = 1; 'c++' },
  h => sub { exists $_[0]->{'c++'} ? 'c++' :'c' },
  csproj => sub { 'xml' },
  cs => sub { 'c#' },
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
  print "#" x $depth, " `$cd`\n\n";
  my $dir_stat = {};
  my @items = grep { !/^\.+$/ } readdir $dh;
  # リポジトリルート下の README.md は最初にそのまま出力する
  if ($depth == 1) {
    @items = grep {
      if ($_ =~ /^readme\.md$/i) {
        open my $fin, "<$cd/$_" or die "ファイル $cd/$_ を開けません";
        while (<$fin>) { chomp; print "$_\n"; }
        print "\n-----\n\n";
        0
      }
      else { 1 }
    } @items;
  }
  # .gitignore があれば情報を収集
  if (any { $_ eq '.gitignore' } @items) {
    open my $fin, "<$cd/.gitignore" or die "ファイル $cd/.gitignore を開けません";
    my @patterns = <$fin>;
    chomp @patterns;
    push @ignore_pattern, { base => $cd, patterns => [@patterns] };
  }

  ITEMS_LOOP:
  for my $file (@items) {
    my $item = "$cd/$file";
    # .gitignore 適用対象を除外
    for my $igpat (@ignore_pattern) {
      my ($base, $patterns) = @$igpat{qw(base patterns)};
      my $is_skip = 0;
      for my $pattern (@$patterns) {
        my $pat = "$pattern";
        my $hit_flag = !($pat =~ /^\~/);
        $pat = $hit_flag ? $pat : substr $pat, 1;
        $pat =~ s/\./\\./g;
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
    # ディレクトリなら再帰
    walk($item, $depth + 1), next if -d $item;
    # fence 情報を決める
    my ($ext) = $file =~ /\.(.*?)$/ or ('');
    next if exists $ng_ext{$ext};
    my $fence = exists $fn2fence{$file} ? $fn2fence{$file}->() : '';
    unless ($fence) {
      for my $p2f (@pat2fence) {
        my ($pat, $fc) = @$p2f{qw(pat fence)};
        $fence = $fc, last if $file =~ /$pat/;
      }
    }
    $fence = $fence || (exists $ext2fence{$ext} ? $ext2fence{$ext}->($dir_stat) : '');
    # Markdown として出力
    open my $fin, "<$item" or die "ファイル $item を開けません";
    print "- $file\n";
    print "  ```$fence\n" unless $fence eq 'markdown';
    my $empty_lines = 0;
    while (<$fin>) {
      chomp;
      my $is_empty = /^\s*$/;
      print $fence eq 'markdown' ? "> $_\n" : "  $_\n" unless $empty_lines && $is_empty;
      $empty_lines = $is_empty;
    }
    print $fence eq 'markdown' ? "\n" : "  ```\n";
  }
}

unless (@ARGV) {
  print STDERR ("収集するソースツリーがあるディレクトリを指定してください。\n");
  exit 1;
}
my $root = $ARGV[0];
$root =~ s!/$!!;

unless (-d $root) {
  print STDERR ("存在するディレクトリを指定してください。\n");
  exit 1;
}

push @ignore_pattern, { base => $root, patterns => ['.git/'] };
walk($root, 1);
