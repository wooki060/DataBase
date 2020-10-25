select name
from Pokemon
where type='Grass'
and id in (
  select before_id
  from Evolution)
