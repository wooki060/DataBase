select lv1 as ID, P1.name, P2.name, P3.name
from (
  select E1.before_id as lv1, E2.before_id as lv2, E2.after_id as lv3
  from Evolution as E1, Evolution as E2
  where E1.before_id not in (
    select after_id
    from Evolution
    )
  and E1.after_id = E2.before_id
  and E2.after_id not in (
    select before_id
    from Evolution
     )
  ) as evo
join Pokemon as P1 on P1.id = evo.lv1
join Pokemon as P2 on P2.id = evo.lv2
join Pokemon as P3 on P3.id = evo.lv3
order by evo.lv1
