--
-- Base de datos: `app_citas`
--


--
-- Estructura de tabla para la tabla `tip_usu`
--

DROP TABLE IF EXISTS `tip_usu`;
CREATE TABLE IF NOT EXISTS `tip_usu` (
  `id_tip_usu` int(11) NOT NULL,
  `nom_tip_usu` varchar(20) NOT NULL,
  PRIMARY KEY (`id_tip_usu`)) 
  ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Volcado de datos para la tabla `tip_usu`
--

INSERT INTO `tip_usu` (`id_tip_usu`, `nom_tip_usu`) VALUES
(1, 'Cedula'),
(2, 'Tarjeta');

-- --------------------------------------------------------

--
-- Estructura de tabla para la tabla `users`
--

DROP TABLE IF EXISTS `users`;
CREATE TABLE IF NOT EXISTS `users` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(20) NOT NULL,
  `email` varchar(100) NOT NULL,
  `password` varchar(100) NOT NULL,
  `id_tip_usu` int(11) NOT NULL,
  `N_id` int(15) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `id_tip_usu` (`id_tip_usu`)) 
  ENGINE=MyISAM AUTO_INCREMENT=25 DEFAULT CHARSET=latin1;
