CREATE TABLE IF NOT EXISTS `accounts` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `name` varchar(16) NOT NULL,
  `password` char(64) NOT NULL,
  `salt` char(12) NOT NULL,
  `realname` varchar(32) NOT NULL,
  `location` varchar(32) NOT NULL,
  `email` varchar(32) NOT NULL,
  `pcname` varchar(32) NOT NULL,
  `hdid` int(10) unsigned NOT NULL,

  PRIMARY KEY(`id`)
);

CREATE TABLE IF NOT EXISTS `characters` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `account` varchar(16) NOT NULL,
  `name` varchar(16) NOT NULL,
  `sex` tinyint(1) NOT NULL DEFAULT '0',
  `race` smallint(1) NOT NULL DEFAULT '0',
  `hairstyle` smallint(2) NOT NULL DEFAULT '1',
  `haircolor` smallint(2) NOT NULL DEFAULT '0',
  `home` varchar(32) DEFAULT 'Wanderer',
  `partner` varchar(16) DEFAULT NULL,
  `title` varchar(16) DEFAULT NULL,
  `exp` bigint NOT NULL DEFAULT '0',
  `level` smallint(3) NOT NULL DEFAULT '0',
  `admin` tinyint(1) NOT NULL DEFAULT '0',
  `map` int NOT NULL DEFAULT '192',
  `x` int NOT NULL DEFAULT '7',
  `y` int NOT NULL DEFAULT '6',
  `direction` smallint NOT NULL DEFAULT '2',
  `hp` int NOT NULL DEFAULT '10',
  `tp` int NOT NULL DEFAULT '10',
  `class` int NOT NULL DEFAULT '1',
  `str` int NOT NULL DEFAULT '0',
  `int` int NOT NULL DEFAULT '0',
  `wis` int NOT NULL DEFAULT '0',
  `agi` int NOT NULL DEFAULT '0',
  `con` int NOT NULL DEFAULT '0',
  `cha` int NOT NULL DEFAULT '0',
  `karma` int NOT NULL DEFAULT '1000',
  `usage` int NOT NULL DEFAULT '0',
  `arena_wins` bigint NOT NULL DEFAULT '0',
  `arena_kills` bigint NOT NULL DEFAULT '0',
  `arena_deaths` bigint NOT NULL DEFAULT '0',
  `sitting` tinyint(1) NOT NULL DEFAULT '0',
  `hidden` tinyint(1) NOT NULL DEFAULT '0',
  `guild` char(3) DEFAULT NULL,
  `guildrank` tinyint(1) NOT NULL DEFAULT '0',
  `guildrank_str` varchar(16) DEFAULT NULL,
  `inventory` text NOT NULL,
  `paperdoll` text NOT NULL,

  PRIMARY KEY(`id`)
);

CREATE TABLE IF NOT EXISTS `guilds` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `tag` char(3) NOT NULL,
  `name` varchar(32) NOT NULL,
  `description` text NOT NULL,
  `ranks` text NOT NULL,
  `wealth` mediumint NOT NULL DEFAULT '10000',

  PRIMARY KEY(`id`)
);

CREATE TABLE IF NOT EXISTS `board_posts` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `board` tinyint(1) NOT NULL,
  `name` varchar(16) NOT NULL,
  `title` varchar(64) NOT NULL,
  `post` text NOT NULL,

  PRIMARY KEY(`id`)
);

CREATE TABLE IF NOT EXISTS `bans` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `ip` int(10) unsigned DEFAULT NULL,
  `hdid` int(10) unsigned DEFAULT NULL,
  `account` varchar(16) DEFAULT NULL,
  `expires` int NOT NULL DEFAULT '0',

  PRIMARY KEY(`id`)
);
