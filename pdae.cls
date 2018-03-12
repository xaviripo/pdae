\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{pdae}[
	2018/02/16 Classe per documents de Programació d'estructures encastades
]
\LoadClass[titlepage]{article}


% paper size
\setlength\paperheight{297mm}
\setlength\paperwidth{210mm}

% font size options
\DeclareOption{10pt}{\renewcommand\@ptsize{0}}
\DeclareOption{11pt}{\renewcommand\@ptsize{1}}
\DeclareOption{12pt}{\renewcommand\@ptsize{2}}

% default options
\ExecuteOptions{a4paper,12pt}

\ProcessOptions

% required packages
\RequirePackage{graphicx}
\RequirePackage{titling}
\RequirePackage{hyperref}
\RequirePackage{fancyhdr}
\RequirePackage[headsep=1.5cm]{geometry}
\RequirePackage{float} % Para que los floats se estén quietecitos
\RequirePackage[utf8]{inputenc}

% short title
\newcommand{\shorttitle}[1]{\def\theshorttitle{#1}}
\shorttitle{\thetitle}

%%% LISTINGS %%%
\RequirePackage{listings}
\RequirePackage{lstautogobble}  % Fix relative indenting
\RequirePackage{color}          % Code coloring
\RequirePackage{zi4}            % Nice font
\RequirePackage{listings}

% Colors %
\definecolor{bluekeywords}{rgb}{0.13, 0.13, 1}
\definecolor{greencomments}{rgb}{0, 0.5, 0}
\definecolor{redstrings}{rgb}{0.9, 0, 0}
\definecolor{graynumbers}{rgb}{0.5, 0.5, 0.5}

\lstset{
    autogobble,
    columns=fullflexible,
    showspaces=false,
    showtabs=false,
    breaklines=true,
    keepspaces=true,
    showstringspaces=false,
    breakatwhitespace=true,
    escapeinside={(*@}{@*)},
    commentstyle=\color{greencomments},
    keywordstyle=\color{bluekeywords},
    stringstyle=\color{redstrings},
    numberstyle=\color{graynumbers},
    basicstyle=\footnotesize\ttfamily,
    frame=l,
    framesep=12pt,
    xleftmargin=12pt,
    tabsize=4,
    captionpos=b
}

\pagestyle{fancy}
\fancyhf{}

% Head
\lhead{\small\theauthor}
\chead{\small Programació d'Arquitectures\\Encastades}
\rhead{\small \theshorttitle}

% Titlepage
\renewcommand{\maketitle}{
\begin{titlepage}
	\centering
  {\scshape\LARGE Universitat de Barcelona \par}
	\vspace{2cm}
	{\scshape\Large Pràctica 3:\par}
	\vspace{1cm}
	{\huge\bfseries \thetitle \par}

  \vfill
  \large\theauthor
	\vfill
	\raggedleft

  \par

  %\hrulefill\par
  {\scshape Programació d'Arquitectures Encastades\par}
  \texttt{}{Curs 2017-2018\par} %Universitat de Barcelona
  \thedate

% Bottom of the page

\end{titlepage}
}
